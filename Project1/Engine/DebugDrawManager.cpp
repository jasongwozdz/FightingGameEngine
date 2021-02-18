#include <math.h>
#include <vector>
#include "DebugDrawManager.h"
#include "GameObjectManager.h"
#include "ResourceManager.h"
//
//std::string sphereModelLoc = "models/geosphere.obj";
//// generate a unit circle on XY-plane
//class Cylinder
//{
//public:
//	std::vector<glm::vec3> vertices;
//	std::vector<Vertex> vert;
//	std::vector<float> normals;
//	glm::vec3 color;
//	std::vector<uint32_t> indices;
//	float height;
//	int sectorCount;
//	float radius;
//	int k1 = 0;                         // 1st vertex index at base
//	int k2 = sectorCount + 1;           // 1st vertex index at top
//	int baseCenterIndex;
//	int topCenterIndex;
//
//	Cylinder(float _height, int _sectorCount, float _radius, glm::vec3 _color, std::vector<Vertex>& vertices, std::vector<uint32_t>& indicies)
//		:
//		height(_height),
//		sectorCount(_sectorCount),
//		radius(_radius),
//		color(_color)
//	{
//		buildVerticesSmooth();
//		buildIndicies();
//		vertices = vert;
//		indicies = indices;
//	}
//
//	std::vector<float> getUnitCircleVertices()
//	{
//		const float PI = 3.1415926f;
//		float sectorStep = 2 * PI / sectorCount;
//		float sectorAngle;  // radian
//
//		std::vector<float> unitCircleVertices;
//		for(int i = 0; i <= sectorCount; ++i)
//		{
//			sectorAngle = i * sectorStep;
//			unitCircleVertices.push_back(cos(sectorAngle)); // x
//			unitCircleVertices.push_back(sin(sectorAngle)); // y
//			unitCircleVertices.push_back(0);                // z
//		}
//		return unitCircleVertices;
//	}
//
//	// generate vertices for a cylinder
//	void buildVerticesSmooth()
//	{
//		// clear memory of prev arrays
//		std::vector<glm::vec3>().swap(vertices);
//		std::vector<float>().swap(normals);
//
//		// get unit circle vectors on XY-plane
//		std::vector<float> unitVertices = getUnitCircleVertices();
//
//		// put side vertices to arrays
//		for(int i = 0; i < 2; ++i)
//		{
//			float h = -height / 2.0f + i * height;           // z value; -h/2 to h/2
//			float t = 1.0f - i;                              // vertical tex coord; 1 to 0
//
//			for(int j = 0, k = 0; j <= sectorCount; ++j, k += 3)
//			{
//				float ux = unitVertices[k];
//				float uy = unitVertices[k+1];
//				float uz = unitVertices[k+2];
//				// position vector]
//				vert.push_back({
//					{ ux * radius, uy * radius, h },
//					{color},
//					{0,0},
//					{ux, uy, uz}
//				});
//			}
//		}
//
//		// the starting index for the base/top surface
//		//NOTE: it is used for generating indices later
//		baseCenterIndex = (int)vertices.size() / 3;
//		topCenterIndex = baseCenterIndex + sectorCount + 1; // include center vertex
//
//		// put base and top vertices to arrays
//		for(int i = 0; i < 2; ++i)
//		{
//			float h = -height / 2.0f + i * height;           // z value; -h/2 to h/2
//			int nz = -1 + i * 2;                           // z value of normal; -1 to 1
//
//			// center point
//			vert.push_back({
//				{0, 0, h},
//				{color},
//				{0,0},
//				{0,0,nz}
//				});
//
//			for(int j = 0, k = 0; j < sectorCount; ++j, k += 3)
//			{
//				float ux = unitVertices[k];
//				float uy = unitVertices[k+1];
//				// position vector
//				vert.push_back({
//					{ ux * radius, uy * radius, h },
//					{color},
//					{0,0},
//					{0, 0, nz}
//				});
//			}
//		}
//	}
//
//	void buildIndicies() 
//	{
//		// indices for the side surface
//		for(int i = 0; i < sectorCount; ++i, ++k1, ++k2)
//		{
//			// 2 triangles per sector
//			// k1 => k1+1 => k2
//			indices.push_back(k1);
//			indices.push_back(k1 + 1);
//			indices.push_back(k2);
//
//			// k2 => k1+1 => k2+1
//			indices.push_back(k2);
//			indices.push_back(k1 + 1);
//			indices.push_back(k2 + 1);
//		}
//
//		// indices for the base surface
//		//NOTE: baseCenterIndex and topCenterIndices are pre-computed during vertex generation
//		//      please see the previous code snippet
//		for(int i = 0, k = baseCenterIndex + 1; i < sectorCount; ++i, ++k)
//		{
//			if(i < sectorCount - 1)
//			{
//				indices.push_back(baseCenterIndex);
//				indices.push_back(k + 1);
//				indices.push_back(k);
//			}
//			else // last triangle
//			{
//				indices.push_back(baseCenterIndex);
//				indices.push_back(baseCenterIndex + 1);
//				indices.push_back(k);
//			}
//		}
//
//		// indices for the top surface
//		for(int i = 0, k = topCenterIndex + 1; i < sectorCount; ++i, ++k)
//		{
//			if(i < sectorCount - 1)
//			{
//				indices.push_back(topCenterIndex);
//				indices.push_back(k);
//				indices.push_back(k + 1);
//			}
//			else // last triangle
//			{
//				indices.push_back(topCenterIndex);
//				indices.push_back(k);
//				indices.push_back(topCenterIndex + 1);
//			}
//		}
//	}
//
//};
//
//DebugDrawManager::DebugDrawManager()
//{
//}
//
//void DebugDrawManager::init()
//{
//	Renderer& renderer = Renderer::getSingleton();
//
//	PipelineManager& pipelineManager = PipelineManager::getSingleton();
//}
//
//DebugDrawManager::~DebugDrawManager()
//{
//}
//
//void DebugDrawManager::addPoint(glm::vec3 pos, glm::vec3 color, float duration, float depthEnabled)
//{
//	Renderer& renderer = Renderer::getSingleton();
//
//	ModelReturnVals	returnVals = ResourceManager::getSingleton().loadObjFile(sphereModelLoc);
//
//	GameObjectManager& manager = GameObjectManager::getSingleton();
//	manager.addGameObject(returnVals.vertices, returnVals.indices, pos, DEBUG_PIPELINE);
//}
//
//void DebugDrawManager::addLine(glm::vec3 fromPos, glm::vec3 toPos, glm::vec3 color, float lineWidth, float duration, bool depthEnabled)
//{
//	GameObjectManager& manager = GameObjectManager::getSingleton();
//
//	std::vector<Vertex> vertices = {
//		{fromPos, color, { 0.0, 0.0 }, { 0.0,0.0,0.0 } },
//		{toPos, color, { 0.0, 0.0 }, { 0.0,0.0,0.0 } },
//	};
//	
//	std::vector<uint32_t> indices = { 0, 1 };
//
//	manager.addGameObject(vertices, indices, { 0,0,0 }, LINE_PIPELINE);
//}
//
//void DebugDrawManager::drawGrid(glm::vec3 color, bool depthEnabled)
//{
//
//	GameObjectManager& manager = GameObjectManager::getSingleton();
//
//	glm::vec3 x1 = { 50, -50, 0 };
//	glm::vec3 x2 = { -50, -50, 0 };
//	glm::vec3 y1 = { -50, 50, 0 };
//	glm::vec3 y2 = { -50, -50, 0 };
//
//	std::vector<Vertex> vertices;
//	std::vector<uint32_t> indices;
//
//	for (int i = 0; i < 101; ++i)
//	{
//		vertices.push_back({ x1,color });
//		vertices.push_back({ x2,color });
//		indices.push_back(i*4);
//		indices.push_back(i*4 + 1);
//
//		vertices.push_back({ y1,color });
//		vertices.push_back({ y2,color });
//		indices.push_back(i*4 + 2);
//		indices.push_back(i*4 + 3);
//		
//		x1.y++;
//		x2.y++;
//		y1.x++;
//		y2.x++;
//	}
//
//	int id = manager.addGameObject(vertices, indices, { 0,0,0 }, LINE_PIPELINE);
//
//	manager.getGameObjectPtrById(id)->setPosition({ 0, 0, 0 });
//	
//	//for (int i = 0; i < 100; ++i)
//	//{
//	//	addLine(x1, x2, color, 1, 1, true);
//	//	addLine(y1, y2, color, 1, 1, true);
//	//	x1.y++;
//	//	x2.y++;
//	//	y1.x++;
//	//	y2.x++;
//
//	//}
//
//}
