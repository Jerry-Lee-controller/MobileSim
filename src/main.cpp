#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace sim {

constexpr double kPi = 3.14159265358979323846;
constexpr double kDegToRad = kPi / 180.0;

template <typename T>
T clampValue(T value, T minVal, T maxVal) {
    if (value < minVal) {
        return minVal;
    }
    if (value > maxVal) {
        return maxVal;
    }
    return value;
}

struct Vec3 {
    double x;
    double y;
    double z;

    Vec3 operator+(const Vec3 &other) const { return {x + other.x, y + other.y, z + other.z}; }
    Vec3 operator-(const Vec3 &other) const { return {x - other.x, y - other.y, z - other.z}; }
    Vec3 operator*(double scalar) const { return {x * scalar, y * scalar, z * scalar}; }
    Vec3 operator/(double scalar) const { return {x / scalar, y / scalar, z / scalar}; }

    Vec3 &operator+=(const Vec3 &other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Vec3 &operator-=(const Vec3 &other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    Vec3 &operator*=(double scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }
};

inline double dot(const Vec3 &a, const Vec3 &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

inline Vec3 cross(const Vec3 &a, const Vec3 &b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

inline double length(const Vec3 &v) { return std::sqrt(dot(v, v)); }

inline Vec3 normalize(const Vec3 &v) {
    const double len = length(v);
    if (len < 1e-6) {
        return {0.0, 0.0, 0.0};
    }
    return v / len;
}

Vec3 rotateX(const Vec3 &v, double radians) {
    const double c = std::cos(radians);
    const double s = std::sin(radians);
    return {v.x, v.y * c - v.z * s, v.y * s + v.z * c};
}

Vec3 rotateY(const Vec3 &v, double radians) {
    const double c = std::cos(radians);
    const double s = std::sin(radians);
    return {v.x * c + v.z * s, v.y, -v.x * s + v.z * c};
}

Vec3 rotateZ(const Vec3 &v, double radians) {
    const double c = std::cos(radians);
    const double s = std::sin(radians);
    return {v.x * c - v.y * s, v.x * s + v.y * c, v.z};
}

Vec3 orientationForward(double yaw, double pitch, double roll) {
    Vec3 forward{0.0, 0.0, 1.0};
    forward = rotateZ(forward, roll);
    forward = rotateX(forward, pitch);
    forward = rotateY(forward, yaw);
    return normalize(forward);
}

Vec3 orientationUp(double yaw, double pitch, double roll) {
    Vec3 up{0.0, 1.0, 0.0};
    up = rotateZ(up, roll);
    up = rotateX(up, pitch);
    up = rotateY(up, yaw);
    return normalize(up);
}

struct Input {
    double throttleDelta{0.0};
    double pitchDelta{0.0};
    double yawDelta{0.0};
    double rollDelta{0.0};
};

struct Ring {
    Vec3 position{};
    double radius{40.0};
    bool passed{false};
};

struct FlightState {
    Vec3 position{0.0, 80.0, 0.0};
    Vec3 velocity{0.0, 0.0, 30.0};
    double yaw{0.0};
    double pitch{0.0};
    double roll{0.0};
    double throttle{0.4};
    double fuel{120.0};
    int score{0};
};

class Simulator {
  public:
    explicit Simulator(std::size_t ringCount)
        : rings_(generateRings(ringCount)), rng_(static_cast<unsigned int>(std::time(nullptr))) {}

    void step(const Input &input, double dt) {
        applyInput(input);
        integrate(dt);
        checkRings();
        clampToGround();
    }

    const FlightState &state() const { return state_; }
    const std::vector<Ring> &rings() const { return rings_; }

  private:
    FlightState state_{};
    std::vector<Ring> rings_;
    std::mt19937 rng_;

    static std::vector<Ring> generateRings(std::size_t count) {
        std::vector<Ring> result;
        result.reserve(count);
        std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
        std::uniform_real_distribution<double> lateral(-220.0, 220.0);
        std::uniform_real_distribution<double> altitude(40.0, 220.0);
        const double spacing = 320.0;

        for (std::size_t i = 0; i < count; ++i) {
            Ring ring;
            ring.position = {lateral(rng), altitude(rng), spacing * static_cast<double>(i + 1)};
            ring.radius = 45.0;
            ring.passed = false;
            result.push_back(ring);
        }
        return result;
    }

    void applyInput(const Input &input) {
        state_.throttle = clampValue(state_.throttle + input.throttleDelta, 0.0, 1.0);
        state_.pitch = clampValue(state_.pitch + input.pitchDelta, -45.0 * kDegToRad, 45.0 * kDegToRad);
        state_.yaw += input.yawDelta;
        state_.roll = clampValue(state_.roll + input.rollDelta, -80.0 * kDegToRad, 80.0 * kDegToRad);
    }

    void integrate(double dt) {
        constexpr double mass = 750.0;                   // kg
        constexpr double thrustPower = 26000.0;          // N
        constexpr double dragCoefficient = 0.04;         // simplified quadratic drag
        constexpr double liftCoefficient = 0.018;        // scales with speed^2
        constexpr double gravity = 9.81;                 // m/s^2
        constexpr double fuelBurnPerSec = 0.25;          // fuel units per second at full throttle
        constexpr double rollYawCoupling = 0.35;         // roll adds slight yawing turn

        const Vec3 forward = orientationForward(state_.yaw, state_.pitch, state_.roll);
        const Vec3 up = orientationUp(state_.yaw, state_.pitch, state_.roll);

        // Basic forces
        const Vec3 thrust = forward * (thrustPower * state_.throttle);
        const double speed = length(state_.velocity);
        const Vec3 drag = state_.velocity * (-dragCoefficient * speed);
        const Vec3 lift = up * (liftCoefficient * speed * speed);
        const Vec3 gravityForce{0.0, -mass * gravity, 0.0};

        // Banked turn: roll causes gradual yaw change to mimic coordinated turns.
        state_.yaw += (state_.roll * rollYawCoupling) * dt;

        const Vec3 acceleration = (thrust + drag + lift + gravityForce) / mass;
        state_.velocity += acceleration * dt;
        state_.position += state_.velocity * dt;

        const double fuelUse = fuelBurnPerSec * state_.throttle * dt;
        state_.fuel = std::max(0.0, state_.fuel - fuelUse);

        if (state_.fuel <= 0.0) {
            state_.throttle = 0.0;
        }
    }

    void clampToGround() {
        if (state_.position.y < 0.0) {
            state_.position.y = 0.0;
            if (state_.velocity.y < 0.0) {
                state_.velocity.y *= -0.2;  // dampen bounce
            }
        }
    }

    void checkRings() {
        for (auto &ring : rings_) {
            if (ring.passed) {
                continue;
            }
            const double distance = length(state_.position - ring.position);
            if (distance <= ring.radius) {
                ring.passed = true;
                state_.score += 100;
            }
        }
    }
};

}  // namespace sim

sim::Input parseInput(const std::string &line) {
    sim::Input input;
    std::istringstream iss(line);
    std::string token;

    while (iss >> token) {
        if (token == "w" || token == "pitch+" || token == "p+") {
            input.pitchDelta += 0.8 * sim::kDegToRad;
        } else if (token == "s" || token == "pitch-" || token == "p-") {
            input.pitchDelta -= 0.8 * sim::kDegToRad;
        } else if (token == "a" || token == "yaw-" || token == "y-") {
            input.yawDelta -= 1.2 * sim::kDegToRad;
        } else if (token == "d" || token == "yaw+" || token == "y+") {
            input.yawDelta += 1.2 * sim::kDegToRad;
        } else if (token == "q" || token == "roll-" || token == "r-") {
            input.rollDelta -= 1.4 * sim::kDegToRad;
        } else if (token == "e" || token == "roll+" || token == "r+") {
            input.rollDelta += 1.4 * sim::kDegToRad;
        } else if (token == "+" || token == "t+" || token == "throttle+") {
            input.throttleDelta += 0.04;
        } else if (token == "-" || token == "t-" || token == "throttle-") {
            input.throttleDelta -= 0.04;
        }
    }

    return input;
}

void printHUD(const sim::Simulator &simulator, int tick, double dt) {
    const auto &state = simulator.state();
    const auto &rings = simulator.rings();

    const int remaining = static_cast<int>(std::count_if(rings.begin(), rings.end(), [](const sim::Ring &r) {
        return !r.passed;
    }));

    std::cout << "\n=== 틱 " << tick << " (" << std::fixed << std::setprecision(1) << dt
              << "s) ===\n";
    std::cout << std::setprecision(2)
              << "위치 (x,y,z): " << state.position.x << ", " << state.position.y << ", "
              << state.position.z << " m\n"
              << "속도: " << sim::length(state.velocity) << " m/s  (전진="
              << sim::dot(sim::normalize(state.velocity),
                          sim::orientationForward(state.yaw, state.pitch, state.roll)) *
                     sim::length(state.velocity)
              << ")\n"
              << "요/피치/롤 (deg): " << state.yaw / sim::kDegToRad << " / "
              << state.pitch / sim::kDegToRad << " / " << state.roll / sim::kDegToRad << "\n"
              << "스로틀: " << state.throttle * 100.0 << "%  연료: " << state.fuel << " u\n"
              << "점수: " << state.score << "  남은 링: " << remaining << "\n";
}

void printHelp() {
    std::cout << "\n입력 방법 (공백으로 여러 명령 동시 입력 가능):\n"
              << "  + 또는 t+ 또는 throttle+ : 스로틀 증가\n"
              << "  - 또는 t- 또는 throttle- : 스로틀 감소\n"
              << "  w / pitch+ / p+          : 기수 올리기 (피치 업)\n"
              << "  s / pitch- / p-          : 기수 내리기 (피치 다운)\n"
              << "  a / yaw- / y-            : 좌선회 (요 -)\n"
              << "  d / yaw+ / y+            : 우선회 (요 +)\n"
              << "  q / roll- / r-           : 좌측 롤\n"
              << "  e / roll+ / r+           : 우측 롤\n"
              << "  help                     : 도움말 다시 보기\n"
              << "  exit                     : 즉시 종료\n";
}

int main() {
    constexpr double dt = 0.1;  // seconds per tick
    sim::Simulator simulator(6);

    std::cout << "간단한 텍스트 기반 비행 시뮬레이터 (C++)\n";
    std::cout << "목표: 연료를 아껴가며 링을 통과해 점수를 얻으세요.\n";
    printHelp();

    int tick = 0;
    std::string line;

    while (simulator.state().fuel > 0.0) {
        printHUD(simulator, tick, dt);
        std::cout << "명령 입력: ";
        std::getline(std::cin, line);
        if (!std::cin) {
            break;
        }
        if (line == "exit") {
            break;
        }
        if (line == "help") {
            printHelp();
            continue;
        }

        const sim::Input input = parseInput(line);
        simulator.step(input, dt);
        ++tick;
    }

    std::cout << "\n비행 종료! 최종 점수: " << simulator.state().score << "\n";
    return 0;
}
