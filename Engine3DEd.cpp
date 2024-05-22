#include "Engine3D.h"

void Engine3D::addModel(shapetype type, glm::vec3 position)
{
	if (type == shapetype::CUBE)
	{
		cube cube(std::max(editingWidth, std::max(editingHeight, editingDepth)), editingRotationX, editingRotationY, editingRotationZ);
		cubeModel mdl(0, cubePointsCnt, cubemapNames[editingCubemapNameIndex], position, cube, editingIsSolid);
		cubePointsCnt += mdl.modelMesh.tris.size() * 3;
		std::cout << "about to place model with sn = " << mdl.sn << std::endl;
		mtx.lock();
		if (modelsInFocus.size() > 0) { modelInFocusTmp = **(modelsInFocus.begin()); }
		//ptrModelsToRender.push_back(std::make_shared<model>(mdl));
		ptrModelsToRender.push_back(std::make_shared<cubeModel>(mdl));
		editingModel = ptrModelsToRender.back();
		std::cout << "placed model has sn = " << editingModel->sn << std::endl;
		mtx.unlock();
	} else
	{
		model m;
		if (type == shapetype::RECTANGLE)
		{
			rectangle rectangle(editingWidth, editingHeight, editingRotationX, editingRotationY, editingRotationZ);
			model mdl(0, modelPointsCnt, textureNames[editingTextureNameIndex], position, rectangle, editingIsSolid);
			m = mdl;
		}else if (type == shapetype::CUBOID)
		{
			cuboid cuboid(editingWidth, editingHeight, editingDepth, editingRotationX, editingRotationY, editingRotationZ);
			model mdl(0, modelPointsCnt, textureNames[editingTextureNameIndex], position, cuboid, editingIsSolid);
			m = mdl;
		}
		modelPointsCnt += m.modelMesh.tris.size() * 3;
		std::cout << "about to place model with sn = " << m.sn << std::endl;
		mtx.lock();
		if (modelsInFocus.size() > 0) { modelInFocusTmp = **(modelsInFocus.begin()); }
		//ptrModelsToRender.push_back(std::make_shared<model>(mdl));
		ptrModelsToRender.push_back(std::make_shared<model>(m));
		editingModel = ptrModelsToRender.back();
		std::cout << "placed model has sn = " << editingModel->sn << std::endl;
		mtx.unlock();
	}
}

void Engine3D::addModel(model& mdl)
{
	if (mdl.modelMesh.shape == shapetype::CUBE)
	{
		mdl.sn = cubePointsCnt;
		cubePointsCnt += mdl.modelMesh.tris.size() * 3;
		std::cout << "about to place model with sn = " << mdl.sn << std::endl;
		mtx.lock();
		if (modelsInFocus.size() > 0) { modelInFocusTmp = **(modelsInFocus.begin()); }
		ptrModelsToRender.push_back(std::make_shared<cubeModel>(mdl));
		editingModel = ptrModelsToRender.back();
		std::cout << "placed model has sn = " << editingModel->sn << std::endl;
		mtx.unlock();
	} else
	{
		mdl.sn = modelPointsCnt;
		modelPointsCnt += mdl.modelMesh.tris.size() * 3;
		std::cout << "about to place model with sn = " << mdl.sn << std::endl;
		mtx.lock();
		if (modelsInFocus.size() > 0) { modelInFocusTmp = **(modelsInFocus.begin()); }
		ptrModelsToRender.push_back(std::make_shared<model>(mdl));
		editingModel = ptrModelsToRender.back();
		std::cout << "placed model has sn = " << editingModel->sn << std::endl;
		mtx.unlock();
	}
}

void Engine3D::removeModel(std::shared_ptr<model> m)
{
	mtx.lock();
	m->removeFlag = true;
	unsigned long i = 0;
	for (i; i < ptrModelsToRender.size(); i++)
	{
		if (ptrModelsToRender[i]->removeFlag) break;
	}
	unsigned long removeIndex = i;
	std::cout << "removing model with index = " << i << " and sn = " << ptrModelsToRender[i]->sn << std::endl;
	if (m->modelMesh.shape == shapetype::CUBE)
	{
		cubePointsCnt -= m->modelMesh.tris.size() * 3;
	}else 
	{
		modelPointsCnt -= m->modelMesh.tris.size() * 3;
	}
	for (i+=1; i < ptrModelsToRender.size(); i++)
	{
		if ((m->modelMesh.shape == shapetype::CUBE && ptrModelsToRender[i]->modelMesh.shape == shapetype::CUBE) ||
			(m->modelMesh.shape != shapetype::CUBE && ptrModelsToRender[i]->modelMesh.shape != shapetype::CUBE))
		{
			ptrModelsToRender[i]->sn -= m->modelMesh.tris.size() * 3;
		}
	}
	ptrModelsToRender[removeIndex].reset();
	ptrModelsToRender.erase(ptrModelsToRender.begin() + removeIndex);
	m.reset();
	modelsInFocus.clear();
	finalModelsToRender.clear();
	finalCubeModelsToRender.clear();
	mtx.unlock();
}

void Engine3D::edit(float elapsedTime)
{
	if (userMode != UserMode::EDITOR) return;

	if (updateVerticesFlag) return;

	if (eventController != nullptr)
	{
		//bool* keysPressed = eventController->getKeysPressed();
		bool isEdited = false;

		// pressing LCTRL + mouse wheel up/down cycles through edit options
		if (keysPressed[SupportedKeys::LEFT_CTRL] && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] && keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] == false) {
			if (--editOptionIndex > editOptions.size() - 1) editOptionIndex = editOptions.size() - 1;
			std::cout << "editing: " << editOptions[editOptionIndex] << std::endl;

		} else if (keysPressed[SupportedKeys::LEFT_CTRL] && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_UP] && keysPressed[SupportedKeys::MOUSE_WHEEL_UP] == false) {
			if (++editOptionIndex > editOptions.size() - 1) editOptionIndex = 0;
			std::cout << "editing: " << editOptions[editOptionIndex] << std::endl;

		// increases/decreases collation height
		} else if (editOptions[editOptionIndex] == "collationHeight" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] && keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] == false) {
			collationHeight = std::max(--collationHeight, (unsigned int)1);
			std::cout << "collation height: " << collationHeight << std::endl;
		} else if (editOptions[editOptionIndex] == "collationHeight" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_UP] && keysPressed[SupportedKeys::MOUSE_WHEEL_UP] == false) {
			collationHeight++;
			std::cout << "collation height: " << collationHeight << std::endl;

		// increases/decreases collation width
		} else if (editOptions[editOptionIndex] == "collationWidth" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] && keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] == false) {
			collationWidth = std::max(--collationWidth, (unsigned int)1);
			std::cout << "collation width: " << collationWidth << std::endl;
		} else if (editOptions[editOptionIndex] == "collationWidth" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_UP] && keysPressed[SupportedKeys::MOUSE_WHEEL_UP] == false) {
			collationWidth++;
			std::cout << "collation width: " << collationWidth << std::endl;

		// cycles through shapes
		} else if (editOptions[editOptionIndex] == "shape" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_UP] && keysPressed[SupportedKeys::MOUSE_WHEEL_UP] == false) {
			if (++edShapeInt > shapetype::CUBE) edShapeInt = 0;
			editingShape = (shapetype)edShapeInt;
			std::cout << "shape: " << shapeTypeToString(editingShape) << std::endl;
		} else if (editOptions[editOptionIndex] == "shape" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] && keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] == false) {
			if (--edShapeInt > shapetype::CUBE) edShapeInt = shapetype::CUBE;
			editingShape = (shapetype)edShapeInt;
			std::cout << "shape: " << shapeTypeToString(editingShape) << std::endl;

		// increases/decreases width
		} else if (editOptions[editOptionIndex] == "width" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] && keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] == false) {
			editingWidth = std::max(editingWidth - 0.1f, 0.1f);
			std::cout << "width: " << editingWidth << std::endl;
		} else if (editOptions[editOptionIndex] == "width" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_UP] && keysPressed[SupportedKeys::MOUSE_WHEEL_UP] == false) {
			editingWidth += 0.1f;
			std::cout << "width: " << editingWidth << std::endl;

		// increases/decreases height
		} else if (editOptions[editOptionIndex] == "height" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] && keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] == false) {
			editingHeight = std::max(editingHeight - 0.1f, 0.1f);
			std::cout << "height: " << editingHeight << std::endl;
		} else if (editOptions[editOptionIndex] == "height" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_UP] && keysPressed[SupportedKeys::MOUSE_WHEEL_UP] == false) {
			editingHeight += 0.1f;
			std::cout << "height: " << editingHeight << std::endl;

		// increases/decreases depth
		} else if (editOptions[editOptionIndex] == "depth" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] && keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] == false) {
			editingDepth = std::max(editingDepth - 0.1f, 0.1f);
			std::cout << "depth: " << editingDepth << std::endl;
		} else if (editOptions[editOptionIndex] == "depth" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_UP] && keysPressed[SupportedKeys::MOUSE_WHEEL_UP] == false) {
			editingDepth += 0.1f;
			std::cout << "depth: " << editingDepth << std::endl;

		// increases/decreases X rotation
		} else if (editOptions[editOptionIndex] == "rotationX" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] && keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] == false) {
			editingRotationX = std::max(editingRotationX - 0.1f, 0.0f);
			std::cout << "rotationX: " << editingRotationX << std::endl;
		} else if (editOptions[editOptionIndex] == "rotationX" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_UP] && keysPressed[SupportedKeys::MOUSE_WHEEL_UP] == false) {
			editingRotationX += 0.1f;
			std::cout << "rotationX: " << editingRotationX << std::endl;

		// increases/decreases Y rotation
		} else if (editOptions[editOptionIndex] == "rotationY" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] && keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] == false) {
			editingRotationY = std::max(editingRotationY - 0.1f, 0.0f);
			std::cout << "rotationY: " << editingRotationY << std::endl;
		} else if (editOptions[editOptionIndex] == "rotationY" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_UP] && keysPressed[SupportedKeys::MOUSE_WHEEL_UP] == false) {
			editingRotationY += 0.1f;
			std::cout << "rotationY: " << editingRotationY << std::endl;

		// increases/decreases Z rotation
		} else if (editOptions[editOptionIndex] == "rotationZ" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] && keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] == false) {
			editingRotationZ = std::max(editingRotationZ - 0.1f, 0.0f);
			std::cout << "rotationZ: " << editingRotationZ << std::endl;
		} else if (editOptions[editOptionIndex] == "rotationZ" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_UP] && keysPressed[SupportedKeys::MOUSE_WHEEL_UP] == false) {
			editingRotationZ += 0.1f;
			std::cout << "rotationZ: " << editingRotationZ << std::endl;

		// cycles through textures
		} else if (editOptions[editOptionIndex] == "texture" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_UP] && keysPressed[SupportedKeys::MOUSE_WHEEL_UP] == false) {
			if (editingShape == shapetype::CUBE)
			{
				if (++editingCubemapNameIndex > cubemapNames.size() - 1) editingCubemapNameIndex = 0;
				std::cout << "cubemap: " << cubemapNames[editingCubemapNameIndex] << std::endl;
			}else
			{
				if (++editingTextureNameIndex > textureNames.size() - 1) editingTextureNameIndex = 0;
				std::cout << "texture: " << textureNames[editingTextureNameIndex] << std::endl;
			}
		} else if (editOptions[editOptionIndex] == "texture" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] && keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] == false) {
			if (editingShape == shapetype::CUBE)
			{
				if (--editingCubemapNameIndex > cubemapNames.size() - 1) editingCubemapNameIndex = cubemapNames.size() - 1;
				std::cout << "cubemap: " << cubemapNames[editingCubemapNameIndex] << std::endl;
			}else
			{
				if (--editingTextureNameIndex > textureNames.size() - 1) editingTextureNameIndex = textureNames.size() - 1;
				std::cout << "texture: " << textureNames[editingTextureNameIndex] << std::endl;
			}

		// toggles isSolid
		} else if (editOptions[editOptionIndex] == "isSolid" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] && keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] == false) {
			editingIsSolid = !editingIsSolid;
			std::cout << "isSolid: " << editingIsSolid << std::endl;
		} else if (editOptions[editOptionIndex] == "isSolid" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_UP] && keysPressed[SupportedKeys::MOUSE_WHEEL_UP] == false) {
			editingIsSolid = !editingIsSolid;
			std::cout << "isSolid: " << editingIsSolid << std::endl;
		}

		if (editingModel == nullptr && keysPressed[SupportedKeys::MOUSE_LEFT_CLICK]) {
			glm::vec3 position = cameraPos + (editingDepth + originalCollidingDistance) * cameraFront;
			addModel(editingShape, position);
			cameraSpeedFactor /= 100;
			isEdited = true;
		}
		
		if (editingModel != nullptr) {
			editingModel->position = cameraPos + (editingDepth + originalCollidingDistance) * cameraFront;
		}

		// releasing left mouse click places a new model
		if (editingModel != nullptr && keysPressed[SupportedKeys::MOUSE_LEFT_CLICK]==false) {
			
			cameraSpeedFactor *= 100;
			axis heightAlongAxis = axis::Y;
			axis widthAlongAxis = axis::X;
			short dirX = cameraFront.x / std::abs(cameraFront.x);
			short dirY = cameraFront.y / std::abs(cameraFront.y);
			short dirZ = cameraFront.z / std::abs(cameraFront.z);

			glm::vec3 pos = glm::vec3(0, 0, 0);
			if (std::abs(cameraFront.z) > std::abs(cameraFront.x)*1.2f && std::abs(cameraFront.z) > std::abs(cameraFront.y)*1.2f)
			{
				heightAlongAxis = axis::Y;
				widthAlongAxis = axis::Z;
				pos.z = dirZ;
			} else if (std::abs(cameraFront.x) > std::abs(cameraFront.y)*1.2f && std::abs(cameraFront.x) > std::abs(cameraFront.z)*1.2f)
			{
				heightAlongAxis = axis::Y;
				widthAlongAxis = axis::X;
				pos.x = dirX;
			} else if (std::abs(cameraFront.y) > std::abs(cameraFront.x)*1.2f && std::abs(cameraFront.y) > std::abs(cameraFront.z)*1.2f)
			{
				if (std::abs(cameraFront.x) > std::abs(cameraFront.z)) {
					heightAlongAxis = axis::X;
					widthAlongAxis = axis::Z;
					pos.z = dirZ;
				} else {
					heightAlongAxis = axis::Z;
					widthAlongAxis = axis::X;
					pos.x = dirX;
				}
			}

			model m = *editingModel;
			bool isGlued = false;
			if (keysPressed[SupportedKeys::LEFT_CTRL] && modelInFocusTmp.inFocus) {
				m.position = modelInFocusTmp.position + editingDepth * pos;
				modelInFocusTmp.inFocus = false;
				removeModel(ptrModelsToRender.back());
				addModel(m);
				isGlued = true;
				std::cout << "glued placement" << std::endl;
			}else {
				editingModel->position = cameraPos + (editingDepth + originalCollidingDistance) * cameraFront;
				std::cout << "standard placement" << std::endl;
			}

			glm::vec3 initialPos = m.position;
			std::cout << "dir X Y Z : " << dirX << ", " << dirY << ", " << dirZ << std::endl;
			//mtx.lock();
			//std::cout << "models size before: " << modelsToRender.size() << std::endl;
			std::cout << "models size before: " << ptrModelsToRender.size() << std::endl;
			for (unsigned int i = 0; i < collationWidth; i++) {
				unsigned int j = i > 0 ? 0 : 1;
				for (j; j < collationHeight; j++) {
					std::cout << "into collation height loop!" << std::endl;
					//std::cout << "repeating for model " << i;
					//m.position.y += editingHeight;
					if (heightAlongAxis == axis::Y) m.position.y += editingHeight;
					else if (heightAlongAxis == axis::X) m.position.x += dirX * editingWidth;
					else if (heightAlongAxis == axis::Z) m.position.z += dirZ *  editingDepth;
					//m.sn = cubePointsCnt;
					//cubePointsCnt += m.modelMesh.tris.size() * 3;
					//ptrModelsToRender.push_back(std::make_shared<cubeModel>(m));
					addModel(m);
				}
				if (widthAlongAxis == axis::Z) m.position.z += dirZ * editingDepth;
				else if (widthAlongAxis == axis::X) m.position.x += dirX * editingWidth;
				else if (widthAlongAxis == axis::Y) m.position.y += editingHeight;
				//m.position.y = initialPos.y - editingHeight;
				if (heightAlongAxis == axis::Y) m.position.y = initialPos.y - editingHeight;
				else if (heightAlongAxis == axis::X) m.position.x = initialPos.x - dirX * editingWidth;
				else if (heightAlongAxis == axis::Z) m.position.z = initialPos.z - dirZ * editingDepth;
			}
			//mtx.unlock();
			//std::cout << "models size after: " << modelsToRender.size() << std::endl;
			std::cout << "models size after: " << ptrModelsToRender.size() << std::endl;

			// std::cout << "model placed has sn: " << editingModel->sn << std::endl;
			editingModel = nullptr;
			isEdited = true;
			//std::cout << "added cube!" << cameraFront.x << cameraFront.y << cameraFront.z << std::endl;

		// right mouse click deletes the model currently in focus
		}else if (deletingModel == nullptr && keysPressed[SupportedKeys::MOUSE_RIGHT_CLICK] && modelsInFocus.size() > 0) {
			mtx.lock();
			auto modelInFocus = *(modelsInFocus.begin());
			deletingModel = modelInFocus;
			mtx.unlock();

		}else if (deletingModel != nullptr && deletingModel->removeFlag==false && keysPressed[SupportedKeys::MOUSE_RIGHT_CLICK]==false) {
			removeModel(deletingModel);
			deletingModel.reset();
			isEdited = true;
		}
		updateVerticesFlag = isEdited;
	}

}
