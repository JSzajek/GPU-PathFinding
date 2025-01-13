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
};

bool InitializeDeviceAndContext()
{
	device = OpenCLUtils::create_device();
	if (!device)
	{
		return false;
	}

	context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
	if (err < 0) {
		perror("Couldn't create a context");
		return false;
	}
    return true;
}

bool InitializeProgram()
{
	/* Build program */
	program = OpenCLUtils::build_program(context, device, "shaders/pathfinding.cl");
	if (!program)
		return false;

	queue = clCreateCommandQueue(context, device, 0, &err);
	if (err < 0)
	{
		perror("Couldn't create a command queue");
		return false;
	};

	/* Create a kernel */
	kernel = clCreateKernel(program, "enemyai", &err);
	if (err < 0)
	{
		perror("Couldn't create a kernel");
		return false;
	};
    return true;
}

int main()
{
	const size_t Num_Entities = 5;
	const cv::Scalar GoalColor(0, 0, 255);
	const cv::Scalar EntityColor(255, 0, 0);

	// Create float-based heightmap and water map
	cv::Mat mapImg = cv::imread("content/test_map.png", cv::IMREAD_GRAYSCALE);

	const int MapWidth = mapImg.cols;
	const int MapHeight = mapImg.rows;

	const size_t GoalRadius = 10;
	const size_t EntityRadius = 5;
	const size_t LineThickness = 2;

	Vector2f GoalPosition(256, 256);

	std::vector<Entity> Entities(Num_Entities);

	// Initialize entities
	for (int i = 0; i < Num_Entities; i++)
	{
		Vector2f newPosition;
		while(true)
		{
			newPosition = Vector2f(RandUtils::RandomRange<float>(0, static_cast<float>(MapWidth - 1)),
								   RandUtils::RandomRange<float>(0, static_cast<float>(MapHeight - 1)));

			if (mapImg.at<uint8_t>(static_cast<size_t>(newPosition.y), 
								   static_cast<size_t>(newPosition.x)) >= 250)
				break;
		}
		Entities[i].position = newPosition;
	}

	if (!InitializeDeviceAndContext())
		return -1;

	if (!InitializeProgram())
		return -1;

	const size_t mapBufferDataSize = MapWidth * MapHeight * sizeof(uint8_t);
	cl_mem mapBuffer = OpenCLUtils::create_input_buffer(context, mapImg.data, mapBufferDataSize);

	const size_t entitiesBufferDataSize = Num_Entities * sizeof(Entity);
	cl_mem entitiesBuffer = OpenCLUtils::create_inout_buffer(context, Entities.data(), entitiesBufferDataSize);

	/* Create kernel arguments */
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &entitiesBuffer);
	err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &mapBuffer);
	err |= clSetKernelArg(kernel, 2, sizeof(int), &MapWidth);
	err |= clSetKernelArg(kernel, 3, sizeof(int), &MapHeight);
	err |= clSetKernelArg(kernel, 5, sizeof(Vector2f), &GoalPosition);

	size_t global = Num_Entities;

	const std::string winName = "GPU AStar Path Finding";
	cv::namedWindow(winName, cv::WINDOW_AUTOSIZE);

	cv::Mat mapImgRGBA;
	cv::cvtColor(mapImg, mapImgRGBA, cv::COLOR_GRAY2BGRA);

	cv::Mat outputImg(MapHeight, MapWidth, mapImgRGBA.type(), cv::Scalar(0));
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

		mapImgRGBA.copyTo(outputImg);

		// Draw goal position
		cv::circle(outputImg, 
				   cv::Point(static_cast<int>(GoalPosition.x), static_cast<int>(GoalPosition.y)),
				   static_cast<int>(GoalRadius),
				   GoalColor,
				   LineThickness, 
				   cv::LineTypes::FILLED);

		for (size_t i = 0; i < Num_Entities; ++i)
		{
			Entity& entity = Entities[i];

			cv::circle(outputImg, 
					   cv::Point(static_cast<int>(entity.position.x), static_cast<int>(entity.position.y)),
					   static_cast<int>(EntityRadius),
					   EntityColor,
					   LineThickness, 
					   cv::LineTypes::FILLED);
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