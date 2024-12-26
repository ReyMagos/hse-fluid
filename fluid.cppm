module;

#include <array>
#include <cassert>
#include <cstring>
#include <ostream>
#include <random>
#include <algorithm>
#include <fstream>

export module fluid;

import lazy_arithmetic;

namespace fluid {
    static constexpr std::array<std::pair<int, int>, 4> deltas{{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};
    std::mt19937 rnd{1337};
    std::uniform_real_distribution dis01{0.0, 1.0};

    template <typename T, size_t N, size_t M>
    struct VectorField {
        std::array<T, deltas.size()> v[N][M];

        T &add(int x, int y, int dx, int dy, T dv) {
            return get(x, y, dx, dy) += dv;
        }

        T &get(int x, int y, int dx, int dy) {
            size_t i = std::ranges::find(deltas, std::pair(dx, dy)) - deltas.begin();
            assert(i < deltas.size());
            return v[x][y][i];
        }
    };

    export template <
        typename pType, typename vType, typename vFlowType,
        size_t N, size_t M
    >
    class Simulation {
    public:
        explicit Simulation(char (&field)[N][M + 1]): field{field} {}

        void render(std::ofstream &out) {
            constexpr size_t T = 1'000'000;
            pType rho[256];
            rho[' '] = 0.01;
            rho['.'] = 1000;
            vType g = 0.1;

            for (size_t x = 0; x < N; ++x) {
                for (size_t y = 0; y < M; ++y) {
                    if (field[x][y] == '#')
                        continue;
                    for (auto [dx, dy] : deltas) {
                        dirs[x][y] += (field[x + dx][y + dy] != '#');
                    }
                }
            }

            for (size_t i = 0; i < T; ++i) {
                bool prop = step(rho, g);

                if (prop) {
                    out.seekp(0);
                    out << "Tick " << i << ":\n";
                    for (size_t x = 0; x < N; ++x) {
                        out << field[x] << "\n";
                    }
                }
            }
        }

    private:
        char (&field)[N][M + 1];
        pType p[N][M]{}, old_p[N][M];
        pType dirs[N][M]{};
        VectorField<vType, N, M> velocity{};
        VectorField<vFlowType, N, M> velocity_flow{};
        int last_use[N][M]{};
        int UT = 0;

        bool step(pType rho[], vType g) {
            pType total_delta_p = 0;

            // Apply external forces
            for (size_t x = 0; x < N; ++x) {
                for (size_t y = 0; y < M; ++y) {
                    if (field[x][y] == '#') {
                        continue;
                    }
                    if (field[x + 1][y] != '#') {
                        velocity.add(x, y, 1, 0, g);
                    }
                }
            }

            // Apply forces from p
            memcpy(old_p, p, sizeof(p));
            for (size_t x = 0; x < N; ++x) {
                for (size_t y = 0; y < M; ++y) {
                    if (field[x][y] == '#') {
                        continue;
                    }

                    for (auto [dx, dy]: deltas) {
                        int nx = x + dx, ny = y + dy;
                        if (field[nx][ny] != '#' && old_p[nx][ny] < old_p[x][y]) {
                            pType force = old_p[x][y] - old_p[nx][ny];
                            vType &contr = velocity.get(nx, ny, -dx, -dy);

                            if (static_cast<pType>(contr * rho[field[nx][ny]]) >= force) {
                                contr -= force / rho[field[nx][ny]];
                                continue;
                            }

                            force -= contr * rho[field[nx][ny]];
                            contr = 0;
                            velocity.add(x, y, dx, dy, static_cast<vType>(force / rho[field[x][y]]));
                            p[x][y] -= force / dirs[x][y];
                            total_delta_p -= force / dirs[x][y];
                        }
                    }
                }
            }

            // Make flow from velocities
            velocity_flow = {};
            bool prop = false;
            do {
                UT += 2;
                prop = false;
                for (size_t x = 0; x < N; ++x) {
                    for (size_t y = 0; y < M; ++y) {
                        if (field[x][y] != '#' && last_use[x][y] != UT) {
                            auto [t, local_prop, _] = propagate_flow(x, y, 1);
                            if (t > 0) {
                                prop = true;
                            }
                        }
                    }
                }
            } while (prop);

            // Recalculate p with kinetic energy
            for (size_t x = 0; x < N; ++x) {
                for (size_t y = 0; y < M; ++y) {
                    if (field[x][y] == '#') {
                        continue;
                    }

                    for (auto [dx, dy]: deltas) {
                        vType     old_v = velocity.get(x, y, dx, dy);
                        vFlowType new_v = velocity_flow.get(x, y, dx, dy);
                        if (old_v > 0) {
                            assert(new_v <= old_v);

                            velocity.get(x, y, dx, dy) = static_cast<vType>(new_v);
                            vType force = (old_v - new_v) * rho[field[x][y]];
                            if (field[x][y] == '.') {
                                force *= 0.8;
                            }

                            if (field[x + dx][y + dy] == '#') {
                                p[x][y] += force / dirs[x][y];
                                total_delta_p += force / dirs[x][y];
                            } else {
                                p[x + dx][y + dy] += force / dirs[x + dx][y + dy];
                                total_delta_p += force / dirs[x + dx][y + dy];
                            }
                        }
                    }
                }
            }

            UT += 2;
            prop = false;
            for (size_t x = 0; x < N; ++x) {
                for (size_t y = 0; y < M; ++y) {
                    if (field[x][y] != '#' && last_use[x][y] != UT) {
                        if (random01() < move_prob(x, y)) {
                            prop = true;
                            propagate_move(x, y, true);
                        } else {
                            propagate_stop(x, y, true);
                        }
                    }
                }
            }

            return prop;
        }

        std::tuple<pType, bool, std::pair<int, int>> propagate_flow(int x, int y, pType lim) {
            last_use[x][y] = UT - 1;
            pType ret = 0;
            for (auto [dx, dy] : deltas) {
                int nx = x + dx, ny = y + dy;
                if (field[nx][ny] != '#' && last_use[nx][ny] < UT) {
                    vType     cap  = velocity.get(x, y, dx, dy);
                    vFlowType flow = velocity_flow.get(x, y, dx, dy);
                    if (flow == cap) {
                        continue;
                    }

                    // assert(v >= velocity_flow.get(x, y, dx, dy));
                    pType vp = std::min(lim, static_cast<pType>(cap - flow));
                    if (last_use[nx][ny] == UT - 1) {
                        velocity_flow.add(x, y, dx, dy, static_cast<vFlowType>(vp));
                        last_use[x][y] = UT;
                        // cerr << x << " " << y << " -> " << nx << " " << ny << " " << vp << " / " << lim << "\n";
                        return {vp, 1, {nx, ny}};
                    }

                    auto [t, prop, end] = propagate_flow(nx, ny, vp);
                    ret += t;
                    if (prop) {
                        velocity_flow.add(x, y, dx, dy, static_cast<vFlowType>(t));
                        last_use[x][y] = UT;
                        // cerr << x << " " << y << " -> " << nx << " " << ny << " " << t << " / " << lim << "\n";
                        return {t, prop && end != std::pair(x, y), end};
                    }
                }
            }
            last_use[x][y] = UT;
            return {ret, 0, {0, 0}};
        }

        void propagate_stop(int x, int y, bool force = false) {
            if (!force) {
                bool stop = true;
                for (auto [dx, dy] : deltas) {
                    int nx = x + dx, ny = y + dy;
                    if (field[nx][ny] != '#' && last_use[nx][ny] < UT - 1 && velocity.get(x, y, dx, dy) > 0) {
                        stop = false;
                        break;
                    }
                }
                if (!stop) {
                    return;
                }
            }

            last_use[x][y] = UT;
            for (auto [dx, dy]: deltas) {
                int nx = x + dx, ny = y + dy;
                if (field[nx][ny] == '#' || last_use[nx][ny] == UT || velocity.get(x, y, dx, dy) > 0) {
                    continue;
                }
                propagate_stop(nx, ny);
            }
        }

        vType move_prob(int x, int y) {
            vType sum = 0;

            for (auto [dx, dy]: deltas) {
                int nx = x + dx, ny = y + dy;
                if (field[nx][ny] == '#' || last_use[nx][ny] == UT) {
                    continue;
                }

                vType v = velocity.get(x, y, dx, dy);
                if (v < 0) {
                    continue;
                }

                sum += v;
            }

            return sum;
        }

        void swap_with(int x, int y) {
            static char type;
            static pType cur_p;
            static std::array<vType, deltas.size()> v;

            std::swap(field[x][y], type);
            std::swap(p[x][y], cur_p);
            std::swap(velocity.v[x][y], v);
        }

        bool propagate_move(int x, int y, bool is_first) {
            last_use[x][y] = UT - is_first;
            bool ret = false;
            int nx = -1, ny = -1;
            do {
                std::array<vType, deltas.size()> tres;
                vType sum = 0;
                for (size_t i = 0; i < deltas.size(); ++i) {
                    auto [dx, dy] = deltas[i];
                    int nx = x + dx, ny = y + dy;
                    if (field[nx][ny] == '#' || last_use[nx][ny] == UT) {
                        tres[i] = sum;
                        continue;
                    }

                    vType v = velocity.get(x, y, dx, dy);
                    if (v < 0) {
                        tres[i] = sum;
                        continue;
                    }

                    sum += v;
                    tres[i] = sum;
                }

                if (sum == 0) {
                    break;
                }

                vType p = random01() * sum;
                size_t d = std::ranges::upper_bound(tres, p) - tres.begin();

                auto [dx, dy] = deltas[d];
                nx = x + dx;
                ny = y + dy;
                assert(velocity.get(x, y, dx, dy) > 0 && field[nx][ny] != '#' && last_use[nx][ny] < UT);

                ret = (last_use[nx][ny] == UT - 1 || propagate_move(nx, ny, false));
            } while (!ret);

            last_use[x][y] = UT;
            for (auto [dx, dy]: deltas) {
                int nx = x + dx, ny = y + dy;
                if (field[nx][ny] != '#' && last_use[nx][ny] < UT - 1 && velocity.get(x, y, dx, dy) < 0) {
                    propagate_stop(nx, ny);
                }
            }

            if (ret && !is_first) {
                swap_with(x, y);
                swap_with(nx, ny);
                swap_with(x, y);
            }

            return ret;
        }

        static vType random01() {
            return static_cast<vType>(dis01(rnd));
        }
    };

    export template <size_t N, size_t M>
    struct run_simulation {
        char (&field)[N][M + 1];
        explicit run_simulation(char (&field)[N][M + 1]): field(field) {}

        template <typename pType, typename vType, typename vFlowType>
        void operator()() const {
            std::ofstream file_output;
            file_output.open("output.txt", std::ios::trunc);

            Simulation<pType, vType, vFlowType, N, M> simulation(field);
            simulation.render(file_output);

            file_output.close();
        }
    };
}
