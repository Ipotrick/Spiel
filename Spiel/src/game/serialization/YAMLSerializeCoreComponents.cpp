#include "YAMLSerializeCoreComponents.hpp"

YAML::Emitter& operator<<(YAML::Emitter& out, const Transform& b)
{
	out << YAML::BeginMap;

	out << YAML::Key << "Position" << YAML::Value << b.position;
	out << YAML::Key << "RotaVec" << YAML::Value << b.rotaVec;

	out << YAML::EndMap;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Movement& b)
{
	out << YAML::BeginMap;

	out << YAML::Key << "Velocity" << YAML::Value << b.velocity;
	out << YAML::Key << "AnlgeVelocity" << YAML::Value << b.angleVelocity;

	out << YAML::EndMap;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const CompountCollider& b)
{
	out << YAML::BeginMap;
	out << YAML::Key << "Size" << YAML::Value << b.size;
	out << YAML::Key << "RelPos" << YAML::Value << b.relativePos;
	out << YAML::Key << "RelRota" << YAML::Value << b.relativeRota;
	out << YAML::Key << "Form" << YAML::Value << b.form;

	out << YAML::EndMap;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Collider& c)
{
	out << YAML::BeginMap;	// Collider 

	out << YAML::Key << "Size" << YAML::Value << c.size;
	out << YAML::Key << "IgnoreMask" << YAML::Value << c.ignoreGroupMask;
	out << YAML::Key << "Mask" << YAML::Value << c.groupMask;
	out << YAML::Key << "ExtraCollider" << YAML::Value << c.extraColliders;
	out << YAML::Key << "particle" << YAML::Value << c.particle;
	out << YAML::Key << "Form" << YAML::Value << c.form;
	out << YAML::Key << "IgnoreTypes";
	out << YAML::BeginMap;	// IgnoreTypes 

	out << YAML::Key << "Dynamic" << YAML::Value << (bool)(c.collisionSettings & Collider::DYNAMIC);
	out << YAML::Key << "Static" << YAML::Value << (bool)(c.collisionSettings & Collider::STATIC);
	out << YAML::Key << "Particle" << YAML::Value << (bool)(c.collisionSettings & Collider::PARTICLE);
	out << YAML::Key << "Sensor" << YAML::Value << (bool)(c.collisionSettings & Collider::SENSOR);

	out << YAML::EndMap;	// IgnoreTypes 
	out << YAML::Key << "MyTypes";
	out << YAML::BeginMap;	// MyTypes 

	out << YAML::Key << "Dynamic" << YAML::Value << (bool)(c.collisionSettings & (Collider::DYNAMIC << 4));
	out << YAML::Key << "Static" << YAML::Value << (bool)(c.collisionSettings & (Collider::STATIC << 4));
	out << YAML::Key << "Particle" << YAML::Value << (bool)(c.collisionSettings & (Collider::PARTICLE << 4));
	out << YAML::Key << "Sensor" << YAML::Value << (bool)(c.collisionSettings & (Collider::SENSOR << 4));

	out << YAML::EndMap;	// MyTypes 
	out << YAML::EndMap;	// Collider 
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const PhysicsBody& b)
{
	out << YAML::BeginMap;	// PhysicsBody 

	out << YAML::Key << "Elasticity" << YAML::Value << b.elasticity;
	out << YAML::Key << "Mass" << YAML::Value << b.mass;
	out << YAML::Key << "MomentInertia" << YAML::Value << b.momentOfInertia;
	out << YAML::Key << "Friction" << YAML::Value << b.friction;

	out << YAML::EndMap;	// PhysicsBody 
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Draw& b)
{
	out << YAML::BeginMap;	// Draw 

	out << YAML::Key << "Color" << YAML::Value << b.color;
	out << YAML::Key << "Scale" << YAML::Value << b.scale;
	out << YAML::Key << "DrawingPrio" << YAML::Value << b.drawingPrio;
	out << YAML::Key << "Form" << YAML::Value << b.form;

	out << YAML::EndMap;	// Draw 
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const LinearEffector& b)
{
	out << YAML::BeginMap;	// LinearEffector 

	out << YAML::Key << "Direction" << YAML::Value << b.direction;
	out << YAML::Key << "Force" << YAML::Value << b.force;
	out << YAML::Key << "Acceleration" << YAML::Value << b.acceleration;

	out << YAML::EndMap;	// LinearEffector 
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const FrictionEffector& b)
{
	out << YAML::BeginMap;	

	out << YAML::Key << "Friction" << YAML::Value << b.friction;
	out << YAML::Key << "RotaFriction" << YAML::Value << b.rotationalFriction;

	out << YAML::EndMap;	
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const TextureName& b)
{
	out << YAML::BeginMap;

	out << YAML::Key << "name" << YAML::Value << b.name;

	out << YAML::EndMap;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const TextureLoadInfo& b)
{
	out << YAML::BeginMap;

	out << YAML::Key << "filepath" << YAML::Value << b.filepath;
	out << YAML::Key << "minFilter" << YAML::Value << cast<u32>(b.minFilter);
	out << YAML::Key << "magFilter" << YAML::Value << cast<u32>(b.magFilter);
	out << YAML::Key << "clampMethod" << YAML::Value << cast<u32>(b.clampMethod);

	out << YAML::EndMap;
	return out;
}
