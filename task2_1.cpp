#include <cstdint>
#include <vector>
#include <array>
#include <cmath>

// === Выравнивание для SIMD ===
struct alignas(16) Vec3 {
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
struct ParticleCInfo {
    Vec3 initialPosition;    Vec3 initialVelocity;
    float mass;
    uint8_t* renderData;
    float brightness;
};

struct alignas(16) Physical {
    Physical(const Vec3& position, const Vec3& velocity, float mass)
        : position(position), velocity(velocity), mass(mass), invMass(1.0f / mass) {}
    
    // do not change the signature
    const Vec3& GetPosition() const { return position; }
    // do not change the signature
    const Vec3& GetVelocity() const { return velocity; }

    Vec3 position;
    Vec3 velocity;
    float mass;
    float invMass;  // === OPT: кэшируем обратную массу ===
};

// do not change
struct Renderable {
    explicit Renderable(const uint8_t* renderData);
    std::array<uint8_t, 1024> data;
};

inline Renderable::Renderable(const uint8_t* /*renderData*/) {
    data.fill(0);
}

struct Glowing {
    // do not change the signature
    float GetBrightness() const { return brightness; }
    
    Glowing() = default;
    explicit Glowing(float b) : brightness(b) {}
    float brightness = 1;
};

struct Particle {
    explicit Particle(const ParticleCInfo& pi)
        : physical(pi.initialPosition, pi.initialVelocity, pi.mass)
        , renderable(pi.renderData)
        , glowing(pi.brightness) {}
    
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
        // === OPT: сырые указатели + pragma для векторизации ===
        const size_t n = physicals.size();
        if (n == 0) return;
        
        Physical* __restrict__ p = physicals.data();
        const float ix = impulse.x, iy = impulse.y, iz = impulse.z;
        
        #pragma GCC ivdep
        for (size_t i = 0; i < n; ++i) {
            const float invM = p[i].invMass;
            p[i].velocity.x += ix * invM;
            p[i].velocity.y += iy * invM;
            p[i].velocity.z += iz * invM;
        }
    }

    void ClearVelocity() {
        const size_t n = physicals.size();
        if (n == 0) return;
        Physical* __restrict__ p = physicals.data();
        
        #pragma GCC ivdep
        for (size_t i = 0; i < n; ++i) {
            p[i].velocity = { 0, 0, 0 };
        }
    }

    // do not change the signature
    void Step(float dt) {
        const size_t n = physicals.size();
        if (n == 0) return;        Physical* __restrict__ p = physicals.data();
        
        #pragma GCC ivdep
        for (size_t i = 0; i < n; ++i) {
            p[i].position.x += p[i].velocity.x * dt;
            p[i].position.y += p[i].velocity.y * dt;
            p[i].position.z += p[i].velocity.z * dt;
        }
    }

    // do not change the signature and the relation with sin
    void StepGlow(float t) {
        const size_t n = glowings.size();
        if (n == 0) return;
        
        const float newBrightness = std::sin(t);
        Glowing* __restrict__ g = glowings.data();
        
        #pragma GCC ivdep
        for (size_t i = 0; i < n; ++i) {
            g[i].brightness = newBrightness;
        }
    }

    std::vector<Physical> physicals;
    std::vector<Renderable> renderables;
    std::vector<Glowing> glowings;
};

int main() {
    constexpr size_t NUM_PARTICLES = 1'000'000;
    constexpr int NUM_STEPS = 1000;
    
    ParticleSystem ps;
    ps.physicals.reserve(NUM_PARTICLES);
    ps.renderables.reserve(NUM_PARTICLES);
    ps.glowings.reserve(NUM_PARTICLES);
    
    std::array<uint8_t, 1024> buf{};
    
    for (size_t i = 0; i < NUM_PARTICLES; ++i) {
        ParticleCInfo info{
            {float(i), float(i), float(i)}, 
            {1.f, 0.5f, 0.25f}, 
            1.f + (i%10)*0.1f, 
            buf.data(), 
            1.f
        };
        ps.CreateParticle(info);
    }    
    for (int s = 0; s < NUM_STEPS; ++s) {
        ps.ApplyImpulse({10.f, 5.f, 2.5f});
        ps.Step(0.016f);
        ps.StepGlow(s * 0.1f);
    }
    return 0;
}
