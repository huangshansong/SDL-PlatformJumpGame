#include <stdio.h>
#include <vector>
#include <string>
#include <ctime>

#include <SDL.h>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "structures.h"

using namespace std;

Mesh* processMesh(aiMesh* mesh, const aiScene* scene)
{
	Mesh* myMesh = new Mesh;

	// data to fill
	vector<Vertex>* vertices = &(myMesh->vertices);
	vector<unsigned int>* indices = &(myMesh->indices);

	vertices->reserve(mesh->mNumVertices);
	indices->reserve(mesh->mNumVertices);

	// walk through each of the mesh's vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
		// positions
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;
		// normals
		if (mesh->HasNormals())
		{
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;
		}
		// texture coordinates
		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			glm::vec2 vec;
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
			/*
			// tangent
			vector.x = mesh->mTangents[i].x;
			vector.y = mesh->mTangents[i].y;
			vector.z = mesh->mTangents[i].z;
			vertex.Tangent = vector;
			// bitangent
			vector.x = mesh->mBitangents[i].x;
			vector.y = mesh->mBitangents[i].y;
			vector.z = mesh->mBitangents[i].z;
			vertex.Bitangent = vector;
			*/
		}
		else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);

		(*vertices).push_back(vertex);
	}
	// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		// retrieve all indices of the face and store them in the indices vector
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			(*indices).push_back(face.mIndices[j]);
	}
	// return a mesh object created from the extracted mesh data
	return myMesh;
}

void processNode(aiNode* node, const aiScene* scene, Model* model)
{
	model->meshes.reserve(model->meshes.size() + node->mNumMeshes);

	// process each mesh located at the current node
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		model->meshes.push_back(processMesh(mesh, scene));
	}
	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene, model);
	}
}

bool IsInFrustrum(glm::vec3& point)
{
	return abs(point.x) <= 1.0f && abs(point.y) <= 1.0f && abs(point.z) <= 1.0f;
}

bool IsInTriangle(glm::vec3& point, TriangleOrdinary& triangle, glm::vec3& area)
{
	area[2] = glm::cross(point - triangle.point[0], triangle.point[1] - triangle.point[0]).z;
	area[0] = glm::cross(point - triangle.point[1], triangle.point[2] - triangle.point[1]).z;
	area[1] = glm::cross(point - triangle.point[2], triangle.point[0] - triangle.point[2]).z;
	if (area[0] < 0.0f && area[1] < 0.0f && area[2] < 0.0f)
		return true;
	else if (area[0] > 0.0f && area[1] > 0.0f && area[2] > 0.0f)
	{
		area[0] = -area[0]; area[1] = -area[1]; area[2] = -area[2];
		return true;
	}
	else
		return false;
}

auto texture(Texture& texture, glm::vec2 texCoords)
{
	texCoords = texCoords * glm::vec2(texture.width, texture.height) - glm::vec2(0.5f, 0.5f);
	if (texCoords.x < 0.0f)
		texCoords.x += texture.width;
	if (texCoords.y < 0.0f)
		texCoords.y += texture.height;
	glm::u16vec2 points[4];
	points[0] = glm::u16vec2((int)texCoords.x, (int)texCoords.y);
	points[1] = glm::u16vec2(((int)texCoords.x + 1) % texture.width, (int)texCoords.y);
	points[2] = glm::u16vec2((int)texCoords.x, ((int)texCoords.y + 1) % texture.height);
	points[3] = glm::u16vec2(((int)texCoords.x + 1) % texture.width, ((int)texCoords.y + 1) % texture.height);

	//二次线性插值
	//现在只处理diffuse, 是RGB24格式，数据为8位整形
	glm::vec3 data[4];
	for (int i = 0; i < 4; i++)
	{
		data[i] = glm::u8vec3(
			*(texture.texture + (texture.nrComponents) * (points[i].y * texture.width + points[i].x)),
			*(texture.texture + (texture.nrComponents) * (points[i].y * texture.width + points[i].x) + 1),
			*(texture.texture + (texture.nrComponents) * (points[i].y * texture.width + points[i].x) + 2)
		);
	}
	glm::vec3 dataTop = data[0] * ( 1.0f - (texCoords.x - points[0].x)) + data[1] * (texCoords.x - points[0].x);
	glm::vec3 dataBottom = data[2] * (1.0f - (texCoords.x - points[2].x)) + data[3] * (texCoords.x - points[2].x);
	glm::vec3 dataFinal = dataTop * (1.0f - (texCoords.y - points[2].y)) + dataBottom * (texCoords.y - points[2].y);

	return dataFinal;
}




//Entry Point
#define main main//因为SDL里重新定义了main所以这里改一下
int main()
{
//建立窗口
	SDL_Rect rect = { 0, 0, 720, 480 };

	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* surface;
	{
		SDL_Init(SDL_INIT_VIDEO);
		window = SDL_CreateWindow(
			"Window",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			rect.w,
			rect.h,
			0
		);
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	}



//初始化
	//中间数据
		//显示在屏幕上的像素
		unsigned char* pixels = (unsigned char*)malloc(sizeof(unsigned char) * 3 * rect.w * rect.h);
		//光栅化后像素所属的三角形
		int* triangleIndexOfPixel = (int*)malloc(sizeof(int) * rect.w * rect.h);
		//深度图
		float* depthMap = (float*)malloc(sizeof(float) * rect.w * rect.h);

	//读入纹理，这些纹理的格式为SDL_PIXELFORMAT_RGB24
	Texture diffuseMap, specularMap, normalMap, roughnessMap, aoMap;
	diffuseMap.texture = stbi_load("C:\\dev\\SoftRasterization\\SoftRasterization\\resource\\objects\\backpack\\diffuse.jpg", &diffuseMap.width, &diffuseMap.height, &diffuseMap.nrComponents, 0);
	specularMap.texture = stbi_load("C:\\dev\\SoftRasterization\\SoftRasterization\\resource\objects\\backpack\\specular.jpg", &specularMap.width, &specularMap.height, &specularMap.nrComponents, 0);
	normalMap.texture = stbi_load("C:\\dev\\SoftRasterization\\SoftRasterization\\resource\\objects\\backpack\\normal.png", &normalMap.width, &normalMap.height, &normalMap.nrComponents, 0);
	roughnessMap.texture = stbi_load("C:\\dev\\SoftRasterization\\SoftRasterization\\resource\\objects\\backpack\\roughness.jpg", &roughnessMap.width, &roughnessMap.height, &roughnessMap.nrComponents, 0);
	aoMap.texture = stbi_load("C:\\dev\\SoftRasterization\\SoftRasterization\\resource\\objects\\backpack\\ao.jpg", &aoMap.width, &aoMap.height, &aoMap.nrComponents, 0);

	//读入模型数据
	// read file via ASSIMP
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile("C:\\dev\\SoftRasterization\\SoftRasterization\\resource\\objects\\backpack\\backpack.obj", aiProcess_Triangulate | aiProcess_GenSmoothNormals);
	// process ASSIMP's root node recursively
	Model* model = new Model;
	processNode(scene->mRootNode, scene, model);
	
	//创建三角形数据
	vector<Triangle> triangles;
	int triangleCount = 0;
	for (int meshIndex = 0; meshIndex < model->meshes.size(); meshIndex++)
	{
		Mesh* mesh = model->meshes[meshIndex];
		for (int i = 0; i < mesh->indices.size(); i += 3)
		{
			triangleCount++;
		}
	}
	triangles.resize(triangleCount);
	triangleCount = 0;
	for (int meshIndex = 0; meshIndex < model->meshes.size(); meshIndex++)
	{
		Mesh* mesh = model->meshes[meshIndex];
		for (int i = 0; i < mesh->indices.size();)
		{
			{
				triangles[triangleCount].point[0] = &(mesh->vertices[mesh->indices[i]]);
				i++;
				triangles[triangleCount].point[1] = &(mesh->vertices[mesh->indices[i]]);
				i++;
				triangles[triangleCount].point[2] = &(mesh->vertices[mesh->indices[i]]);
				i++;
				triangleCount++;
			}
		}
	}
	//三角形插值权重所用到的面积
	vector<vector<glm::vec3>> areas;
	areas.resize(rect.h);
	for (vector<glm::vec3>& a : areas)
	{
		a.resize(rect.w);
	}


	float timePre = clock() / CLOCKS_PER_SEC;
	float deltatTime;


	//渲染循环
	while (true)
	{
		unsigned int timeCur = clock() / CLOCKS_PER_SEC;
		deltatTime = timeCur - timePre;
		timePre = timeCur;

		//清空中间数据
		for (int i = 0; i < rect.w * rect.h; i++)
		{
			*(depthMap + i) = -1.0f;
			*(triangleIndexOfPixel + i) = -1;
		}

		//MVP矩阵
		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//model = glm::rotate(model, deltatTime * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));帧率太低转不动╥_╥
		glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)rect.w / rect.h, 0.1f, 100.0f);


		//渲染
		for (int i = 0; i < triangles.size(); i++)
		{
		//对三角形做处理
			//VS
			glm::vec3 NDC[3];
			for (int p = 0; p < 3; p++)
			{
				glm::vec4 temp = projection * view * model * glm::vec4(triangles[i].point[p]->Position, 1.0f);
				NDC[p] = temp / temp.w;
			}
			//背面剔除
			if (glm::cross(NDC[1] - NDC[0], NDC[2] - NDC[1]).z < 0.0f)
				continue;
			//平截头体剔除
			if ((!IsInFrustrum(NDC[0])) && (!IsInFrustrum(NDC[1])) && (!IsInFrustrum(NDC[2])))
				continue;
			//光栅化
			TriangleOrdinary triangleDC;
			glm::vec4 minMaxXY = glm::vec4(rect.w, 0.0f, rect.h, 0.0f);
			for (int p = 0; p < 3; p++)
			{
				triangleDC.point[p] = ((NDC[p] + glm::vec3(1.0f, 1.0f, 0.0f)) * glm::vec3(0.5f, 0.5f, 1.0f)) * glm::vec3(rect.w, rect.h, 1.0f);
				minMaxXY[0] = min(minMaxXY[0], triangleDC.point[p].x);
				minMaxXY[1] = max(minMaxXY[1], triangleDC.point[p].x);
				minMaxXY[2] = min(minMaxXY[2], triangleDC.point[p].y);
				minMaxXY[3] = max(minMaxXY[3], triangleDC.point[p].y);
			}
			for (int y = min(rect.h - 1, max(0, (int)(minMaxXY[2] - 0.5f))); y <= min(rect.h - 1, (int)(minMaxXY[3] - 0.5f)); y++)
			{
				for (int x = min(rect.w - 1, max(0, (int)(minMaxXY[0] - 0.5f))); x <= min(rect.w - 1, (int)(minMaxXY[1] - 0.5f)); x++)
				{
					glm::vec3 area;
					if (IsInTriangle(glm::vec3(x + 0.5f, y + 0.5f, 0.0f), triangleDC, area))
					{
		//对像素做处理
						//early-z
						float depth = 0.0f;
						for (int p = 0; p < 3; p++)
						{
							depth += NDC[p].z * area[p] / (area[0] + area[1] + area[2]);
						}
						if (depth < *(depthMap + y * rect.w + x))
							continue;
						*(depthMap + y * rect.w + x) = depth;
						*(triangleIndexOfPixel + y * rect.w + x) = i;
						areas[y][x] = area;
					}
				}
			}
		}
		//贴图采样
		for (int y = 0; y < rect.h; y++)
		{
			for (int x = 0; x < rect.w; x++)
			{
				//printf("%d %d\n", x, y);
				unsigned int i = *(triangleIndexOfPixel + y * rect.w + x);
				if (i != -1)
				{
					glm::vec2 texCoords =
						(
							triangles[i].point[0]->TexCoords * areas[y][x][0]
							+ triangles[i].point[1]->TexCoords * areas[y][x][1]
							+ triangles[i].point[2]->TexCoords * areas[y][x][2]
						)
						/ 
						(areas[y][x][0] + areas[y][x][1] + areas[y][x][2]);

					auto diffuse = texture(diffuseMap, texCoords);


					*(pixels + 3 * (y * rect.w + x)) = (unsigned int)diffuse.x;
					*(pixels + 3 * (y * rect.w + x) + 1) = (unsigned int)diffuse.y;
					*(pixels + 3 * (y * rect.w + x) + 2) = (unsigned int)diffuse.z;


					//*(pixels + 3 * (y * rect.w + x)) = 0;
					//*(pixels + 3 * (y * rect.w + x) + 1) = 0;
					//*(pixels + 3 * (y * rect.w + x) + 2) = i % 255;
				}
				else
				{
					//背景设为黑色
					*(pixels + 3 * (y * rect.w + x)) = 0;
					*(pixels + 3 * (y * rect.w + x) + 1) = 0;
					*(pixels + 3 * (y * rect.w + x) + 2) = 0;
				}
			}
		}
		
		//PS

		//后处理

		//显示
		surface = SDL_CreateRGBSurfaceWithFormatFrom(pixels, rect.w, rect.h, 24, sizeof(unsigned char) * 3 * rect.w, SDL_PIXELFORMAT_RGB24);

		SDL_BlitSurface(surface, &rect, SDL_GetWindowSurface(window), &rect);

		SDL_UpdateWindowSurface(window);

		//SDL_Delay(20);
	}




//收尾
	{
	stbi_image_free(diffuseMap.texture);
	stbi_image_free(specularMap.texture);
	stbi_image_free(normalMap.texture);
	stbi_image_free(roughnessMap.texture);
	stbi_image_free(aoMap.texture);
	std::free(pixels);
	std::free(triangleIndexOfPixel);
	std::free(depthMap);
	SDL_FreeSurface(surface);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);

	SDL_Quit();
	}

	return 0;
}