#include "OpenCLUtils.h"
#include "OpenCVUtils.h"
#include "RandomUtils.h"
#include "Timer.h"

cl_device_id device = nullptr;
cl_context context = nullptr;
cl_program program = nullptr;
cl_kernel kernel = nullptr;
cl_command_queue queue = nullptr;
cl_int err = -1;

struct Vector2f
{
public:
	Vector2f() = default;

	Vector2f(float _x, float _y)
		: x(_x),
		y(_y)
	{
	}
public:
	inline float Length() const { return std::sqrt(x * x + y * y); }
public:
	float x = 0;
	float y = 0;
};

struct Entity
{
public:
	Vector2f position;
	Vector2f velocity;
	Vector2f target_position;
	float acceleration = 1.0f;
	float max_speed = 1.0f;
	int radius = 1;
};

Vector2f FindValidPosition(const cv::Mat& map,
						   uint8_t obstacleThresholdValue = 255)
{
	Vector2f newPosition;

	const int width = map.cols;
	const int height = map.rows;

	while(true)
	{
		newPosition = Vector2f(RandUtils::RandomRange<float>(0, static_cast<float>(width - 1)),
							   RandUtils::RandomRange<float>(0, static_cast<float>(height - 1)));

		if (map.at<uint8_t>(static_cast<int>(newPosition.y), static_cast<int>(newPosition.x)) >= obstacleThresholdValue)
			break;
	}
	return newPosition;
}

void DrawToMap(cv::Mat& map, 
			   const cv::Mat& sprite, 
			   const Vector2f& position)
{
	int x = static_cast<int>(position.x) - static_cast<int>(sprite.cols / 2.0f);
	int y = static_cast<int>(position.y) - static_cast<int>(sprite.rows / 2.0f);

	// Calculate the overlapping region
	int roiX = std::max(0, x);
	int roiY = std::max(0, y);
	int roiWidth = std::min(map.cols - roiX, sprite.cols - std::max(0, -x));
	int roiHeight = std::min(map.rows - roiY, sprite.rows - std::max(0, -y));

	// Check if the ROI has a valid size
	if (roiWidth <= 0 || roiHeight <= 0) 
	{
		std::cerr << "No overlapping region between the sprite and base image!" << std::endl;
		return;
	}

	// Extract the overlapping region from both images
	cv::Mat spriteROI		= sprite(cv::Rect(std::max(0, -x), std::max(0, -y), roiWidth, roiHeight));
	cv::Mat baseImageROI	= map(cv::Rect(roiX, roiY, roiWidth, roiHeight));

	// Handle alpha channel (transparency)
	if (sprite.channels() == 4)
	{
		// Blend the sprite with the base image using the alpha channel
		for (int row = 0; row < spriteROI.rows; ++row)
		{
			for (int col = 0; col < spriteROI.cols; ++col)
			{
				cv::Vec4b spritePixel = sprite.at<cv::Vec4b>(row, col);
				cv::Vec4b& basePixel = baseImageROI.at<cv::Vec4b>(row, col);

				uchar alpha = spritePixel[3];

				// Blend pixels
				basePixel[0] = (basePixel[0] * (255 - alpha) / 255.0f) + (spritePixel[0] * (alpha / 255.0f));
				basePixel[1] = (basePixel[1] * (255 - alpha) / 255.0f) + (spritePixel[1] * (alpha / 255.0f));
				basePixel[2] = (basePixel[2] * (255 - alpha) / 255.0f) + (spritePixel[2] * (alpha / 255.0f));
			}
		}
	}
	else 
	{
		// Copy sprite directly (no alpha channel)
		spriteROI.copyTo(baseImageROI);
	}
}

int main()
{
	srand(time(nullptr));

	const size_t Num_Entities = 25;
	const cv::Scalar GoalColor(0, 0, 255);
	const cv::Scalar EntityColor(36, 53, 18);
	const cv::Scalar TargetColor(255, 255, 0);

	const size_t GoalRadius = 10;
	const size_t EntityRadius = 16;
	const size_t LineThickness = 2;

	const float EntityAcceleration = 0.6f;
	const float EntityMaxSpeed = 5.0f;

	const bool DebugTargetPosition = false;

	cv::Mat shipImg = cv::imread("content/satellite_D.png", cv::IMREAD_UNCHANGED);
	cv::Mat stationImg = cv::imread("content/station_A.png", cv::IMREAD_UNCHANGED);

	cv::resize(shipImg, shipImg, cv::Size(EntityRadius * 2, EntityRadius * 2));

	// Create float-based heightmap and water map
	cv::Mat boundsMapImg = cv::imread("content/bounds_map.png", cv::IMREAD_GRAYSCALE);
	cv::Mat colorMapImg = cv::imread("content/color_map.png");
	if (!OpenCVUtils::ConvertType(colorMapImg, CV_8UC4))
	{
		printf("Failed to Convert Output Image Type!");
		return -1;
	}

	const int MapWidth = boundsMapImg.cols;
	const int MapHeight = boundsMapImg.rows;

	Vector2f GoalPosition = FindValidPosition(boundsMapImg, 250);

	// Initialize entities
	std::vector<Entity> Entities(Num_Entities);
	for (int i = 0; i < Num_Entities; i++)
	{
		Vector2f newPosition = FindValidPosition(boundsMapImg, 250);

		Entities[i].position = newPosition;
		Entities[i].target_position = newPosition;
		Entities[i].radius = EntityRadius;
		Entities[i].acceleration = EntityAcceleration;
		Entities[i].max_speed = EntityMaxSpeed;
	}

	if (!OpenCLUtils::initialize_device_and_context(device, context))
		return -1;

	if (!OpenCLUtils::initialize_program("shaders/heuristic.cl", "enemyai", context, device, program, kernel, queue))
	{
		assert(false);
		return -1;
	}

	const size_t mapBufferDataSize = MapWidth * MapHeight * sizeof(uint8_t);
	cl_mem mapBuffer = OpenCLUtils::create_input_buffer(context, boundsMapImg.data, mapBufferDataSize);

	const size_t entitiesBufferDataSize = Num_Entities * sizeof(Entity);
	cl_mem entitiesBuffer = OpenCLUtils::create_inout_buffer(context, Entities.data(), entitiesBufferDataSize);

	/* Create kernel arguments */
	err = clSetKernelArg(kernel,	0, sizeof(cl_mem), &entitiesBuffer);
	err |= clSetKernelArg(kernel,	1, sizeof(cl_mem), &mapBuffer);
	err |= clSetKernelArg(kernel,	2, sizeof(int), &MapWidth);
	err |= clSetKernelArg(kernel,	3, sizeof(int), &MapHeight);
	err |= clSetKernelArg(kernel,	5, sizeof(Vector2f), &GoalPosition);
	if (err < 0)
	{
		perror("Couldn't create a kernel argument");
		return false;
	}

	size_t global = Num_Entities;

	const std::string winName = "GPU Heuristic Path Finding";
	cv::namedWindow(winName, cv::WINDOW_AUTOSIZE);

	cv::Mat outputImg(MapHeight, MapWidth, colorMapImg.type(), cv::Scalar(0));
	cv::imshow(winName, outputImg);

	Timer gpuBufferReadTimer;
	Timer drawTimer;

	float deltaTime_s = 0.01f;
	while (true)
	{
		gpuBufferReadTimer.Start();

		// Update delta time --------------------------------------------------
		err = clSetKernelArg(kernel, 4, sizeof(float), &deltaTime_s);
		if (err < 0)
		{
			perror("Couldn't create a kernel argument");
			return false;
		}
		// --------------------------------------------------------------------

		err = clEnqueueNDRangeKernel(queue,
									 kernel,
									 1,
									 NULL,
									 (const size_t*)&global,
									 NULL,
									 0,
									 NULL,
									 NULL);

		if (err < 0)
		{
			perror("Couldn't enqueue the kernel");
			return false;
		}

		///* Read the kernel's output    */
		err = clEnqueueReadBuffer(queue,
								  entitiesBuffer,
								  CL_FALSE,
								  0,
								  entitiesBufferDataSize,
								  Entities.data(),
								  0,
								  NULL,
								  NULL);

		if (err < 0)
		{
			perror("Couldn't read the buffer");
			return false;
		}

		clFinish(queue);

		const double gpuBufferTime_ms = gpuBufferReadTimer.Elapsed_ms();

		// Visualization logic
		drawTimer.Start();

		colorMapImg.copyTo(outputImg);

		// Draw goal position
		DrawToMap(outputImg, stationImg, GoalPosition);

		for (size_t i = 0; i < Num_Entities; ++i)
		{
			Entity& entity = Entities[i];

			Vector2f directionVector(entity.position.x - entity.target_position.x,
									 entity.position.y - entity.target_position.y);

			DrawToMap(outputImg, shipImg, entity.position);

			if (DebugTargetPosition)
			{
				cv::circle(outputImg, 
						   cv::Point(static_cast<int>(entity.target_position.y), static_cast<int>(entity.target_position.x)),
						   static_cast<int>(2),
						   TargetColor,
						   LineThickness, 
						   cv::LineTypes::FILLED);
			}
		}

		cv::imshow(winName, outputImg);

		// Press 'ESC' to exit
		if (cv::waitKey(1) == 27)
		{
			break;
		}

		const double drawTime_ms = drawTimer.Elapsed_ms();

		std::cout << "GPU Read Time: " << std::to_string(gpuBufferTime_ms) << "\tDraw Time: " << std::to_string(drawTime_ms) << std::endl;
		deltaTime_s = static_cast<float>(gpuBufferTime_ms + drawTime_ms) * 0.01f; // Convert back to seconds
	}
}