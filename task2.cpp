#include <cstdint>
#include <vector>
#include <array>
#include <cmath>

struct Vec3 {
    // do not remove the default constructor
    Vec3() = default;

    // do not remove the following getters
    float& X() { return x; }
    float& Y() { return y; }
    float& Z() { return z; }
    float X() const { return x; }
    float Y() const { return y; }
    float Z() const { return z; }

    float x;
    float y;
    float z;

    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator + (const Vec3& other) const {
        return { x + other.x, y + other.y, z + other.z };
    }

    Vec3 operator * (float scale) const {
        return { x * scale, y * scale, z * scale };
    }

    Vec3 operator / (float scale) const {
        return { x / scale, y / scale, z / scale };
    }
    
    // === ДОБАВЛЕНО: compound-операторы для оптимизации ===
    Vec3& operator += (const Vec3& other) {
        x += other.x; y += other.y; z += other.z;
        return *this;
    }
    
    Vec3& operator *= (float scale) {
        x *= scale; y *= scale; z *= scale;
        return *this;
    }
};

// do not change
struct ParticleCInfo {    Vec3 initialPosition;
    Vec3 initialVelocity;
    float mass;

    uint8_t* renderData;

    float brightness;
};

struct Physical {
    Physical(const Vec3& position, const Vec3& velocity, float mass)
        : position(position)
        , velocity(velocity)
        , mass(mass)
    {
    }
    // do not change the signature
    const Vec3& GetPosition() const { return position; }

    // do not change the signature
    const Vec3& GetVelocity() const { return velocity; }

    Vec3 position;
    Vec3 velocity;
    float mass;
};

// do not change
struct Renderable {
    explicit Renderable(const uint8_t* renderData);

    std::array<uint8_t, 1024> data;
};

struct Glowing {

    // do not change the signature
    float GetBrightness() const { return brightness; }
    
    // === ДОБАВЛЕНО: конструкторы для совместимости с vector ===
    Glowing() = default;
    explicit Glowing(float b) : brightness(b) {}

    float brightness = 1;
};

struct Particle {
    explicit Particle(const ParticleCInfo& pi)
        : physical(pi.initialPosition, pi.initialVelocity, pi.mass)
        , renderable(pi.renderData)        , glowing(pi.brightness)
    {
    }

    Physical physical;
    Renderable renderable;
    Glowing glowing;
};

struct ParticleSystem {

    // do not change the signature
    const Physical& GetPhysical(size_t id) const { return physicals[id]; }

    // do not change the signature
    const Renderable& GetRenderable(size_t id) const { return renderables[id]; }

    // do not change the signature
    const Glowing& GetGlowing(size_t id) const { return glowings[id]; }
    
    // do not change the signature
    size_t CreateParticle(const ParticleCInfo& pi) {
        physicals.emplace_back(pi.initialPosition, pi.initialVelocity, pi.mass);
        renderables.emplace_back(pi.renderData);
        glowings.emplace_back(pi.brightness);
        return physicals.size() - 1;
    }

    // do not change the signature
    void ApplyImpulse(const Vec3& impulse) {
        // === OPTIMIZED: работаем только с компактным вектором Physical ===
        for (auto& p : physicals) {
            const Vec3 dv = impulse / p.mass;
            p.velocity += dv;
        }
    }

    void ClearVelocity() {
        for (auto& p : physicals) {
            p.velocity = { 0, 0, 0 };
        }
    }

    // do not change the signature
    void Step(float dt) {
        for (auto& p : physicals) {
            p.position += p.velocity * dt;
        }
    }
    // do not change the signature and the relation with sin
    void StepGlow(float t) {
        // === OPTIMIZED: вычисляем sin один раз ===
        const float newBrightness = std::sin(t);
        for (auto& g : glowings) {
            g.brightness = newBrightness;
        }
    }

    // === OPTIMIZED: Structure of Arrays вместо Array of Structures ===
    std::vector<Physical> physicals;
    std::vector<Renderable> renderables;
    std::vector<Glowing> glowings;
};
