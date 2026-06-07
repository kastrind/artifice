#include "CompoundModel.h"

std::set<uint64_t> CompoundModel::loadedIds;

void CompoundModel::save(std::string modelPath)
{
	if (modelPath.empty()) {
		modelPath = this->modelPath;
	}
	printf( "Saving compound model %s\n",  modelPath.c_str() );

	std::ofstream f(modelPath);
	if (!f.is_open())
	{
		printf( "Failed to save model!\n" );
		return;
	}
	else
	{
		f << "# metadata compoundModelId" << std::endl;
		f << "metadata," << id << std::endl;
		f << "# for compound model heads:" << std::endl;
		f << "# id compoundModel scaleX scaleY scaleZ rotationX rotationY rotationZ positionX positionY positionZ compoundModelId" << std::endl;
		f << "# for primitive models (shapes):" << std::endl;
		f << "# id shape texture width height depth isSolid rotationX rotationY rotationZ positionX positionY positionZ" << std::endl;

		bool is_first = true;
		glm::vec3 first_position(0.0f);

		for (auto &ptrModel : models) // to avoid confusion, models can be rectangles, cuboids or cubes, and are parts of the whole compound model
		{
			model& m = *ptrModel;

			// skip, because only head models of compound models need to be saved
			if (m.compoundModelId > 0 && !m.isHeadModel)
			{
				continue;
			}
			// for compound models: saves head models of compound models
			else if (m.compoundModelId > 0 && m.isHeadModel)
			{
				float thetaRotX = atan2(-m.rotationMatrix[2][1], m.rotationMatrix[2][2]);
				float thetaRotY = atan2(m.rotationMatrix[2][0], sqrt(m.rotationMatrix[2][1] * m.rotationMatrix[2][1] + m.rotationMatrix[2][2] * m.rotationMatrix[2][2]));
				float thetaRotZ = atan2(-m.rotationMatrix[1][0], m.rotationMatrix[0][0]);
				f << m.id << ",compoundModel," << "1,1,1," << thetaRotX << "," << thetaRotY << "," << thetaRotZ;
				if (is_first) { // first model position is always 0, 0, 0
					first_position = m.position;
					f << ",0,0,0," << m.compoundModelId << std::endl;
					is_first = false;
				}else { // else position is relative to first
					f << "," << m.position.x - first_position.x << "," << m.position.y - first_position.y << "," << m.position.z - first_position.z << "," << m.compoundModelId << std::endl;
				}
				continue;
			}

			f << m.id << ",";

			if (m.modelMesh.shape == shapetype::CUBE)
			{
				cubeModel& cm = dynamic_cast<cubeModel &>(m);
				float size = cm.modelMesh.tris[0].p[0].x - cm.modelMesh.tris[2].p[0].x;
				f << ((cm.isSkyBox) ? "skybox" : "cube") << "," << cm.texture << "," << size << "," << size << "," << size;
			}
			else if (m.modelMesh.shape == shapetype::CUBOID)
			{
				float width = m.modelMesh.tris[0].p[0].x - m.modelMesh.tris[1].p[0].x;
				float height = m.modelMesh.tris[0].p[0].y - m.modelMesh.tris[0].p[2].y;
				float depth = m.modelMesh.tris[0].p[0].z - m.modelMesh.tris[2].p[0].z;
				f << "cuboid," << m.texture << "," << width << "," << height << "," << depth;

			}
			else if (m.modelMesh.shape == shapetype::RECTANGLE)
			{
				float width = m.modelMesh.tris[0].p[1].x - m.modelMesh.tris[0].p[0].x;
				float height = m.modelMesh.tris[0].p[0].y - m.modelMesh.tris[0].p[2].y;
				f << "rectangle," << m.texture << "," << width << "," << height << "," << "0";
			}
			float thetaRotX = atan2(-m.rotationMatrix[2][1], m.rotationMatrix[2][2]);
			float thetaRotY = atan2(m.rotationMatrix[2][0], sqrt(m.rotationMatrix[2][1] * m.rotationMatrix[2][1] + m.rotationMatrix[2][2] * m.rotationMatrix[2][2]));
			float thetaRotZ = atan2(-m.rotationMatrix[1][0], m.rotationMatrix[0][0]);
			f << "," << (m.isSolid ? "true" : "false") << "," << thetaRotX << "," << thetaRotY << "," << thetaRotZ;

			if (is_first) { // first model position is always 0, 0, 0
				first_position = m.position;
				f << ",0,0,0" << std::endl;
				is_first = false;
			}else { // else position is relative to first
				f << "," << m.position.x - first_position.x << "," << m.position.y - first_position.y << "," << m.position.z - first_position.z << std::endl;
			}
		}

		//TODO: save lights of the model too
		// f << "# id point_light positionX positionY positionZ colorR colorG colorB diffuseIntensity specularIntensity constant linear quadratic" << std::endl;
		// for (PointLight& pl : pointLights)
		// {
		// 	// point lights without id are ignored, like the unlit one which is added if no point lights exist
		// 	if (pl.id == 0) {
		// 		continue;
		// 	}
		// 	f << pl.id << "," << "point_light," << pl.position.x << "," << pl.position.y << "," << pl.position.z << "," << pl.color.r * 255 << "," << pl.color.g * 255 << "," << pl.color.b * 255 << "," << pl.diffuseIntensity << "," << pl.specularIntensity << "," << pl.constant << "," << pl.linear << "," << pl.quadratic << std::endl;
		// }
		// f << "# id spot_light positionX positionY positionZ colorR colorG colorB diffuseIntensity specularIntensity constant linear quadratic dirX dirY dirZ cutoff outerCutoff" << std::endl;
		// for (SpotLight& sl : spotLights)
		// {
		// 	// spot lights without id are ignored, like the unlit one which is added if no spot lights exist
		// 	if (sl.id == 0) {
		// 		continue;
		// 	}
		// 	f << sl.id << "," << "spot_light," << sl.position.x << "," << sl.position.y << "," << sl.position.z << "," << sl.color.r * 255 << "," << sl.color.g * 255 << "," << sl.color.b * 255 << "," << sl.diffuseIntensity << "," << sl.specularIntensity << "," << sl.constant << "," << sl.linear << "," << sl.quadratic << "," << sl.direction.x << "," << sl.direction.y << "," << sl.direction.z << "," << sl.cutoff << "," << sl.outerCutoff << std::endl;
		// }
	}
	f.close();
}

void CompoundModel::load(std::string modelPath, Transform* transform)
{
	this->modelPath = modelPath;
	printf( "Loading compound model %s\n",  modelPath.c_str() );

	std::ifstream f(modelPath);
	if (!f.is_open())
	{
		printf( "Failed to load model!\n" );
		return;
	}
	models.clear();
	Level tempLevel;
	tempLevel.modelPointsCnt = modelPointsCnt;
	tempLevel.cubePointsCnt = cubePointsCnt;
	tempLevel.deserializeModels(f, models);
	id = tempLevel.meta.compoundModelId;
	std::cout << "loaded compoundmodelid: " << id << std::endl;
	if (models.size() > 0)
	{
		models[0]->compoundModelId = id;
		models[0]->isHeadModel = true;
		models[0]->rotate(transform->rotation.x, transform->rotation.y, transform->rotation.z);
		models[0]->modelMatrix = glm::translate(glm::mat4(1.0f), transform->position) * models[0]->rotationMatrix;
		models[0]->modelMatrix = glm::scale(models[0]->modelMatrix, transform->scale);
		models[0]->headModelScale = transform->scale;
		models[0]->position = models[0]->modelMatrix[3];
		models[0]->isTouched = true;
	}
	if (models.size() > 1)
	{
		for (size_t i = 1; i < models.size(); ++i)
		{
			models[i]->compoundModelId = id;
			models[i]->headModel = models[0];
			models[i]->localOffset = models[i]->position;
			models[i]->modelMatrix = models[0]->modelMatrix * glm::translate(glm::mat4(1.0f), models[i]->localOffset) * models[i]->rotationMatrix;
			models[i]->position = models[i]->modelMatrix[3];
			models[i]->isTouched = true;
		}
	}
	modelPointsCnt = tempLevel.modelPointsCnt;
	cubePointsCnt = tempLevel.cubePointsCnt;
}

std::shared_ptr<CompoundModel> CompoundModel::create(std::string modelPath, Transform* transform, uint64_t modelPointsCntOffset, uint64_t cubePointsCntOffset, uint64_t id)
{
	CompoundModel compModel(modelPointsCntOffset, cubePointsCntOffset, id);
	if (!registerLoadedId(id)) { // circular dependency protection
		throw std::runtime_error("Circular dependency detected while loading compoound model " + std::to_string(id) + " from path " + modelPath);
	}
	compModel.load(modelPath, transform);
	unregisterLoadedId(id); // circular dependency protection pt2
	return std::make_shared<CompoundModel>(compModel);
}

bool CompoundModel::registerLoadedId(uint64_t id) {
	if (id>0) {
		auto result = loadedIds.insert(id);
		bool inserted = result.second;
		// std::cout << "Inserted? " << std::boolalpha << inserted << ", contents: ";
		// for (const auto& value : loadedIds) {
		// 	std::cout << value << " ";
		// }
		// std::cout << std::endl;
		return inserted;
	}
	return true;
}

void CompoundModel::unregisterLoadedId(uint64_t id) {
	loadedIds.erase(id);
	// std::cout << "Erased " << id << " from set." << std::endl;
}
