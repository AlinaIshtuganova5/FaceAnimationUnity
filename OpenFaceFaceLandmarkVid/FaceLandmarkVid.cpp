
// FaceTrackingVid.cpp : Определяет точку входа для консольного приложения для отслеживания лиц в видео.

// Библиотеки для обнаружение лицевого ориентира и отслеживание взгляда
#include "LandmarkCoreIncludes.h"
#include "GazeEstimation.h"

#include <fstream>
#include <sstream>

// OpenCV 
#include <opencv2/videoio/videoio.hpp>  // Запись видео
#include <opencv2/videoio/videoio_c.h>  // Запись видео
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <SequenceCapture.h>
#include <Visualizer.h>
#include <VisualizationUtils.h>

// Библиотека Boost 
#include <filesystem.hpp>
#include <filesystem/fstream.hpp>


#define INFO_STREAM( stream ) \
std::cout << stream << std::endl

#define WARN_STREAM( stream ) \
std::cout << "Warning: " << stream << std::endl

#define ERROR_STREAM( stream ) \
std::cout << "Error: " << stream << std::endl

static void printErrorAndAbort( const std::string & error )
{
    std::cout << error << std::endl;
    abort();
}

#define FATAL_STREAM( stream ) \
printErrorAndAbort( std::string( "Fatal error: " ) + stream )

using namespace std;

// Глобальные переменные

bool detection_success=FALSE;
cv::Vec6d pose_estimate;
cv::Point3f gazeDirection0(0, 0, -1);
cv::Point3f gazeDirection1(0, 0, -1);
double g_doubleArray[140];

// DLL 

extern "C" __declspec(dllexport) int __stdcall getXY(void** ppDoubleArrayReceiver)
{
	*ppDoubleArrayReceiver = (void*)g_doubleArray;
	return 0;
}

extern "C"
{
	__declspec(dllexport) int __stdcall getdetection_success() {
		return detection_success;
	}
}

extern "C"
{
	__declspec(dllexport) float __stdcall get_pose1()
	{
		return pose_estimate[3];
	}
}

extern "C"
{
	__declspec(dllexport) float __stdcall get_pose2()
	{
		return pose_estimate[4];
	}
}

extern "C"
{
	__declspec(dllexport) float __stdcall get_pose3()
	{
		return pose_estimate[5];
	}
}

extern "C"
{
	__declspec(dllexport) float __stdcall get_gaze1()
	{
		return gazeDirection0.x;
	}
}
extern "C"
{
	__declspec(dllexport) float __stdcall get_gaze2()
	{
		return gazeDirection0.y;
	}
}
extern "C"
{
	__declspec(dllexport) float __stdcall get_gaze3()
	{
		return gazeDirection0.z;
	}
}
extern "C"
{
	__declspec(dllexport) float __stdcall get_gaze4()
	{
		return gazeDirection1.x;
	}
}
extern "C"
{
	__declspec(dllexport) float __stdcall get_gaze5()
	{
		return gazeDirection1.y;
	}
}
extern "C"
{
	__declspec(dllexport) float __stdcall get_gaze6()
	{
		return gazeDirection1.z;
	}
}

vector<string> get_arguments(int argc, char **argv)
{

	vector<string> arguments;

	for(int i = 0; i < argc; ++i)
	{
		arguments.push_back(string(argv[i]));
	}
	return arguments;
}

// Главная Функция

extern "C"
{
	__declspec(dllexport) int __stdcall main ()
{
	int argcc = 3;
	char* argvv[4];



	argvv[0] = "E:\\Projects\\Face\\FaceLandmarkVid.exe";
	argvv[1] = "-device";
	argvv[2] = "0";
	argvv[3] = NULL;

	vector<string> arguments = get_arguments(argcc, argvv);

	// без аргументов: использование вывода
	if (arguments.size() == 1)
	{
		cout << "For command line arguments see:" << endl;
		cout << " https://github.com/TadasBaltrusaitis/OpenFace/wiki/Command-line-arguments";
		return 0;
	}

	LandmarkDetector::FaceModelParameters det_parameters(arguments);

	// Модули, которые используются для отслеживания
	LandmarkDetector::CLNF face_model(det_parameters.model_location);

	
	Utilities::SequenceCapture sequence_reader;

	// Утилита для визуализации результатов 
	Utilities::Visualizer visualizer(true, false, false);

	// Отслеживание FPS для визуализации
	Utilities::FpsTracker fps_tracker;
	fps_tracker.AddFrame();

	int sequence_number = 0;

	while (true) 
	{

	
		if (!sequence_reader.Open(arguments))
			break;

		INFO_STREAM("Device or file opened");
		cv::Mat captured_image = sequence_reader.GetNextFrame();
		INFO_STREAM("Starting tracking");
		while (!captured_image.empty()) 
		{

			// читает изображение
			cv::Mat_<uchar> grayscale_image = sequence_reader.GetGrayFrame();

			// обнаружение / отслеживание лицевого ориентира
			detection_success = LandmarkDetector::DetectLandmarksInVideo(grayscale_image, face_model, det_parameters);

			// отслеживание вгляда
			// Если отслеживание прошло успешно, и у нас есть модель глаза, оцените взгляд
			if (detection_success && face_model.eye_model)
			{
				GazeAnalysis::EstimateGaze(face_model, gazeDirection0, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy, true);
				GazeAnalysis::EstimateGaze(face_model, gazeDirection1, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy, false);

				// Заполняем позицию XY

				for (int i = 0; i < 136; i++) {
					g_doubleArray[i] = face_model.detected_landmarks.at<double>(i);
				}

				

				//cout << "t[0] : " << g_doubleArray[0] << "\n";
				//cout << "t[10] : " << g_doubleArray[10] << "\n";
				//cout << "t[100] : " << g_doubleArray[100] << "\n";
				//cout << "pts : " << face_model.detected_landmarks.at<double>(0) << "\n";
			}

			// отслеживание позы головы у модели
			pose_estimate = LandmarkDetector::GetPose(face_model, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy);

			// подсчитка FPS
			fps_tracker.AddFrame();

			// визуализация отслеживания
			visualizer.SetImage(captured_image, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy);
			visualizer.SetObservationLandmarks(face_model.detected_landmarks, face_model.detection_certainty, face_model.GetVisibilities());
			visualizer.SetObservationPose(pose_estimate, face_model.detection_certainty);
			visualizer.SetObservationGaze(gazeDirection0, gazeDirection1, LandmarkDetector::CalculateAllEyeLandmarks(face_model), LandmarkDetector::Calculate3DEyeLandmarks(face_model, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy), face_model.detection_certainty);
			visualizer.SetFps(fps_tracker.GetFPS());
			
			char character_press = visualizer.ShowObservation();

			// перезапустить трекер
			if (character_press == 'r')
			{
				face_model.Reset();
			}
			// выход из приложения
			else if (character_press == 'q')
			{
				face_model.Reset();
				return(0);
			}

			// захватить следующий кадр
			captured_image = sequence_reader.GetNextFrame();

		}
		
		// Reset the model, for the next video
		face_model.Reset();
		sequence_number++;

	}
	return 0;
}
}
