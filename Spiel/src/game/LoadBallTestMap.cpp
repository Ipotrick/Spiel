#include "LoadBallTestMap.hpp"

void loadBallTestMap(Game& game)
{
	auto& world = game.world;
	auto makeWall = [&](float x, float y) {
		auto wall = world.create();
		auto comp = world.componentView(wall);
		comp.add<Transform>(Transform(Vec2(x, y), 0));
		comp.add<Draw>(Draw(Vec4(0.8, 0.8, 0.8, 1), Vec2(1.2, 1.2), 0.45f, Form::Rectangle));
		auto coll = Collider(Vec2(1.2, 1.2), Form::Rectangle);
		coll.setIgnore(Collider::DYNAMIC);
		comp.add<Collider>(coll);
		comp.add<PhysicsBody>(PhysicsBody(0.0f, 10000000000000000000000000000000000.0f, 1000000000000000000000000000000000.0f, 1));
		world.spawn(wall);
	};

	world.physics.friction = 0.25f;
	world.physics.linearEffectAccel = 3.0;
	world.physics.linearEffectDir = Vec2(0, -1);


	int const height = 80;
	int const width = 32;
	std::string map = {
		"################################"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                   #######    #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                      ##      #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                         ###  #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                    ###       #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"#                              #"
		"################################"
	};

	for (float vert = 0; vert < height - 0.001f; vert++) {
		for (float hor = 0; hor < width - 0.001f; hor++) {
			if (map.at(uint32_t(vert * width + hor)) == '#') {
				//for (int i = 0; i < 100; i++)
				makeWall(hor, height - vert);
			}
		}
	}

	//for (int i = 0; i < 100'000; i++) {
	//	auto dummy = world.create();
	//	world.addComp(dummy, Transform({ rand() % 100000 / 100.0f, rand() % 100000 / 100.0f }, 0));
	//	world.addComp(dummy, Draw({ rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, 0.1 }, {0.5,0.5}, 0.1, Form::Circle));
	//	world.spawn(dummy);
	//}

	Vec2 scalePlayer(1, 1);
	auto player = world.create();
	auto cmps = world.componentView(player);
	cmps.add<Transform>(Transform(Vec2(2, 12), 0));
	auto colliderPlayer = Collider(Vec2(0.4, 0.7), Form::Rectangle);
	colliderPlayer.extraColliders.push_back(CompountCollider(Vec2(1, 1) * 0.4, Vec2(0, 0.35), RotaVec2(0), Form::Circle));
	colliderPlayer.extraColliders.push_back(CompountCollider(Vec2(0.3, 0.2), Vec2(0.2, -0.3), RotaVec2(305.0f), Form::Rectangle));
	colliderPlayer.extraColliders.push_back(CompountCollider(Vec2(0.3, 0.2), Vec2(-0.2, -0.3), RotaVec2(55.0f), Form::Rectangle));
	cmps.add(colliderPlayer);
	cmps.add(PhysicsBody(0.0, 25.0f, calcMomentOfIntertia(25.0f, scalePlayer), 0.9f));
	cmps.add<Movement>();
	cmps.add(Draw(Vec4(1, 1, 1, 1), scalePlayer, 0.4, Form::Rectangle));
	cmps.add(TextureLoadInfo{ "ressources/bitch.png" });
	cmps.add<Player>();
	world.identify(player);
	world.spawn(player);

	Vec2 scaleBox(16.0f / 9.0f, 1);
	auto box = world.create();
	auto cmpsBox = world.componentView(box);
	cmpsBox.add<Transform>(Transform(Vec2(2, 2), 0));
	auto colliderBox = Collider(scalePlayer, Form::Rectangle);
	colliderBox.groupMask |= CollisionGroup<1>::mask;
	cmpsBox.add(colliderBox);
	cmpsBox.add(PhysicsBody(0.0, 5.0f, calcMomentOfIntertia(5.0f, scaleBox), 0.9f));
	cmpsBox.add<Movement>();
	cmpsBox.add(Draw(Vec4(1, 1, 1, 1), scaleBox, 0.4, Form::Rectangle));
	cmpsBox.add(TextureLoadInfo{"ressources/Dir.png", TexFilter::Nearest, TexFilter::Nearest});
	cmpsBox.add(Health(100));
	world.spawn(box);

	Vec2 scale = Vec2(0.2f, 0.2f);
	Form form = Form::Circle;
	Collider trashCollider = Collider(scale, form);
	PhysicsBody trashSolidBody = PhysicsBody(0.2f, 0.5f, calcMomentOfIntertia(0.5, scale), 0.9f);
	for (int i = 0; i < 10000; i++) {
		//if (i % 2) {
		//	form = Form::Rectangle;
		//	scale = { 0.2f, 0.4f }; 
		//	trashCollider = Collider(scale, form);
		//}
		//else {

		float factor = (rand() % 1000) / 600.0f + 0.7f;
		auto sscale = scale * factor;
		form = Form::Circle;
		trashCollider = Collider(sscale, form);
		//}
		trashCollider.form = form;
		Vec4 color = Vec4(rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, 1);
		Vec2 position = { static_cast<float>(rand() % 1001 / 300.0f) * 4.6f + 5.5f, static_cast<float>(rand() % 1000 / 100.0f) * 4.6f + 5.5f };
		auto trash = world.create();
		world.addComp(trash, Transform(position, RotaVec2(0)));
		world.addComp(trash, Movement());
		world.addComp(trash, trashCollider);
		world.addComp(trash, trashSolidBody);
		world.addComp(trash, Draw(color, sscale, 0.5f, form));
		world.addComp(trash, Health(100));
		world.addComp(trash, TextureLoadInfo{ "ressources/Dir.png" });
		world.spawn(trash);
	}

	//for (int i = 0; i < 50000; i++) {
	//	//if (i % 2) {
	//	//	form = Form::Rectangle;
	//	//	scale = { 0.2f, 0.4f }; 
	//	//	trashCollider = Collider(scale, form);
	//	//}
	//	//else {
	//
	//	float factor = 0.1f;// (rand() % 1000) / 500.0f + 0.7f;
	//	auto sscale = scale * factor;
	//	form = Form::Circle;
	//	trashCollider = Collider(sscale * 0.1f, form);
	//	//}
	//	trashCollider.form = form;
	//	Vec4 color = Vec4(rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, 1);
	//	//Vec2 position = Vec2(5, 1.6 + i * 0.301f);
	//	Vec2 position = { static_cast<float>(rand() % 1001 / 300.0f) * 2.6f + 1.5f, static_cast<float>(rand() % 1000 / 300.0f) * 2.6f + 1.5f };
	//	auto trash = world.create();
	//	world.addComp(trash, Transform(position, RotaVec2(0)));
	//	world.addComp(trash, Movement());
	//	world.addComp(trash, trashCollider);
	//	world.addComp(trash, trashSolidBody);
	//	world.addComp(trash, Draw(color, sscale, 0.5f, form));
	//	world.addComp(trash, Health(100));
	//	world.addComp(trash, TextureDescriptor{ "ressources/Dir.png" });
	//	world.spawn(trash);
	//}

	auto spawner = world.create();
	auto cmps2 = world.componentView(spawner);
	cmps2.add<Transform>(Transform(Vec2(20, 40), 0));
	cmps2.add<Collider>(Collider(Vec2(0.3, 0.3), Form::Circle));
	cmps2.add<PhysicsBody>(PhysicsBody(0.0f, 
		100000000000000000000000000000000.0f, 
		100000000000000000000000000000000.0f, 0));
	cmps2.add<Draw>(Draw{ Vec4{1,0,0,1} });
	cmps2.add<SpawnerComp>();
	world.spawn(spawner);

	auto sucker = world.create();
	auto cmps3 = world.componentView(sucker);
	cmps3.add<Transform>(Transform(Vec2(20, 2), 0));
	auto coll = Collider(Vec2(7, 7), Form::Circle);
	cmps3.add<Collider>(coll);
	cmps3.add<Draw>(Draw(Vec4(0, 0, 1, 1), Vec2(7, 7), 0.4f, Form::Circle));
	auto suckerCmd = SuckerComp();
	suckerCmd.spawner = world.identify(spawner);
	cmps3.add<SuckerComp>(suckerCmd);
	world.spawn(sucker);
}
