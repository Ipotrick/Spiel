#pragma once

#include "YAMLSerialize.hpp"

template<> constexpr bool isYAMLSerializable<PhysicsUniforms>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const PhysicsUniforms& v);
namespace YAML {
    template<>
    struct convert<PhysicsUniforms> {
        static Node encode(const PhysicsUniforms& rhs)
        {
            Node node;
            return node;
        }

        static bool decode(const Node& node, PhysicsUniforms& rhs)
        {
            if (node.size() != 4) {
                return false;
            }

            rhs.friction = node["friction"].as<float>();
            rhs.linearEffectAccel = node["linearEffectAccel"].as<float>();
            rhs.linearEffectDir = node["linearEffectDir"].as<Vec2>();
            rhs.linearEffectForce = node["linearEffectForce"].as<float>();

            return true;
        }
    };
}

template<> constexpr bool isYAMLSerializable<Player>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const Player& v);
namespace YAML {
    template<>
    struct convert<Player> {
        static Node encode(const Player& rhs)
        {
            Node node;
            return node;
        }

        static bool decode(const Node& node, Player& rhs)
        {
            if (node.size() != 3) {
                return false;
            }

            rhs.bulletShotLapTimer = node["BullletShotFrequency"].as<LapTimer>();
            rhs.flameSpawnTimer = node["FireSpwanFrequency"].as<LapTimer>();
            rhs.flameSpawnTimer = node["Power"].as<float>();

            return true;
        }
    };
}

template<> constexpr bool isYAMLSerializable<ParticleScriptComp>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const ParticleScriptComp& v);
namespace YAML {
    template<>
    struct convert<ParticleScriptComp> {
        static Node encode(const ParticleScriptComp& rhs)
        {
            Node node;
            return node;
        }

        static bool decode(const Node& node, ParticleScriptComp& rhs)
        {
            if (node.size() != 5) {
                return false;
            }

            rhs.startSize = node["StartSize"].as<Vec2>();;
            rhs.endSize = node["EndSize"].as<Vec2>();
            rhs.startColor = node["StartColor"].as<Vec4>();
            rhs.endColor = node["EndColor"].as<Vec4>();
            rhs.collisionCount = node["CollisionCount"].as<int>();

            return true;
        }
    };
}

template<> constexpr bool isYAMLSerializable<Health>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const Health& v);
namespace YAML {
    template<>
    struct convert<Health> {
        static Node encode(const Health& rhs)
        {
            Node node;
            return node;
        }

        static bool decode(const Node& node, Health& rhs)
        {

            rhs.maxHealth = node["maxHealth"].as<int>();
            rhs.curHealth = node["curHealth"].as<int>();

            return true;
        }
    };
}

template<> constexpr bool isYAMLSerializable<Age>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const Age& v);
namespace YAML {
    template<>
    struct convert<Age> {
        static Node encode(const Age& rhs)
        {
            Node node;
            return node;
        }

        static bool decode(const Node& node, Age& rhs)
        {
            if (node.size() != 2) {
                return false;
            }

            rhs.maxAge = node["maxAge"].as<float>();
            rhs.curAge = node["curAge"].as<float>();

            return true;
        }
    };
}

template<> constexpr bool isYAMLSerializable<Bullet>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const Bullet& v);
namespace YAML {
    template<>
    struct convert<Bullet> {
        static Node encode(const Bullet& rhs)
        {
            Node node;
            return node;
        }

        static bool decode(const Node& node, Bullet& rhs)
        {
            if (node.size() != 2) {
                return false;
            }

            rhs.damage = node["damage"].as<int>();
            rhs.hitPoints = node["hitPoints"].as<int>();

            return true;
        }
    };
}

template<> constexpr bool isYAMLSerializable<Tester>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const Tester& v);
namespace YAML {
    template<>
    struct convert<Tester> {
        static Node encode(const Tester& rhs)
        {
            Node node;
            return node;
        }

        static bool decode(const Node& node, Tester& rhs)
        {
            if (node.size() != 1) {
                return false;
            }

            rhs.changeDirTime = node["changeDirTime"].as<float>();

            return true;
        }
    };
}

template<> constexpr bool isYAMLSerializable<SuckerComp>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const SuckerComp& v);
namespace YAML {
    template<>
    struct convert<SuckerComp> {
        static Node encode(const SuckerComp& rhs)
        {
            Node node;
            return node;
        }

        static bool decode(const Node& node, SuckerComp& rhs)
        {
            if (node.size() != 1) {
                return false;
            }

            rhs.spawner = node["spawner"].as<UUID>();

            return true;
        }
    };
}

template<> constexpr bool isYAMLSerializable<SpawnerComp>() { return true; }
YAML::Emitter& operator<<(YAML::Emitter& out, const SpawnerComp& v);
namespace YAML {
    template<>
    struct convert<SpawnerComp> {
        static Node encode(const SpawnerComp& rhs)
        {
            Node node;
            return node;
        }

        static bool decode(const Node& node, SpawnerComp& rhs)
        {
            return true;
        }
    };
}