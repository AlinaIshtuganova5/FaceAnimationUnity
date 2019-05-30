using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System;
using System.Text;

public class SetWeights : MonoBehaviour
{
    int blendShapeCount;
    SkinnedMeshRenderer skinnedMeshRenderer;
    Mesh skinnedMesh;
    public Transform head;
    public Transform lefteye;
    public Transform righteye;
    public Transform ltarget;
    public Transform rtarget;
    Vector3 hr;

    // открывается камера getdetection_success fns
    [DllImport("FaceLandmarkVid.dll", EntryPoint = "getdetection_success", CallingConvention = CallingConvention.Cdecl)]
    static extern int getdetection_success();
    [DllImport("FaceLandmarkVid.dll", EntryPoint = "main", CallingConvention = CallingConvention.StdCall)]
    static extern int main();

    // получаем данные головы
    [DllImport("FaceLandmarkVid.dll", EntryPoint = "get_pose1", CallingConvention = CallingConvention.Cdecl)]
    static extern float get_pose1();
    [DllImport("FaceLandmarkVid.dll", EntryPoint = "get_pose2", CallingConvention = CallingConvention.Cdecl)]
    static extern float get_pose2();
    [DllImport("FaceLandmarkVid.dll", EntryPoint = "get_pose3", CallingConvention = CallingConvention.Cdecl)]
    static extern float get_pose3();

    // получаем данные взгляда
    [DllImport("FaceLandmarkVid.dll", EntryPoint = "get_gaze1", CallingConvention = CallingConvention.Cdecl)]
    static extern float get_gaze1();
    [DllImport("FaceLandmarkVid.dll", EntryPoint = "get_gaze2", CallingConvention = CallingConvention.Cdecl)]
    static extern float get_gaze2();
    [DllImport("FaceLandmarkVid.dll", EntryPoint = "get_gaze3", CallingConvention = CallingConvention.Cdecl)]
    static extern float get_gaze3();
    [DllImport("FaceLandmarkVid.dll", EntryPoint = "get_gaze4", CallingConvention = CallingConvention.Cdecl)]
    static extern float get_gaze4();
    [DllImport("FaceLandmarkVid.dll", EntryPoint = "get_gaze5", CallingConvention = CallingConvention.Cdecl)]
    static extern float get_gaze5();
    [DllImport("FaceLandmarkVid.dll", EntryPoint = "get_gaze6", CallingConvention = CallingConvention.Cdecl)]
    static extern float get_gaze6();

    // получаем  X Y положение головы (double array) | 2D ориентир [x1;x2;...xn;y1;y2...yn] 
    [DllImport("FaceLandmarkVid.dll", CallingConvention = CallingConvention.StdCall)]
    public static extern int getXY(out IntPtr pArrayOfDouble);
    double[] ArrayOfDouble = new double[140];

    // поток открытия камеры 
    System.Threading.Thread newThread = new System.Threading.Thread(AMethod);
    private static void AMethod()
    {
        main();
    }

    // инициализация
    void Awake()
    {
        skinnedMeshRenderer = GetComponent<SkinnedMeshRenderer>();
        skinnedMesh = GetComponent<SkinnedMeshRenderer>().sharedMesh;
    }

    // инициализация
    void Start()
    {
        newThread.Start();
        InvokeRepeating("setBlendShapes", 0, 0.033f);
    }

    void Update()
    {
    }

   
    private void setBlendShapes()
    {

        double[] ArrayOfDouble = new double[140];
        double[] skinn = new double[50] { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        
        if (getdetection_success() == 1)
        {
            // Анимация рта и бровей 

           

            IntPtr p2DDoubleArray = IntPtr.Zero;
            getXY(out p2DDoubleArray);
            Marshal.Copy(p2DDoubleArray, ArrayOfDouble, 0, 140);

            skinn[35] = Math.Abs(ArrayOfDouble[51]- ArrayOfDouble[57])*10;


            
            blendShapeCount = skinnedMesh.blendShapeCount;
            for (int i = 0; i < blendShapeCount; i++)
             {
                 skinnedMeshRenderer.SetBlendShapeWeight(i, (float)skinn[i]);
             }

            // анимация головы

            hr = new Vector3(Mathf.Rad2Deg * get_pose1(), Mathf.Rad2Deg * get_pose2(), Mathf.Rad2Deg * get_pose3());
        }
        else
        {
            hr = new Vector3(0, 0, 0);
        }
        head.rotation = Quaternion.Euler(hr);

        // анимация взгляда

        ltarget.localPosition = new Vector3(-1 * (get_gaze1() - 0.13f), get_gaze2() - 0.1f, get_gaze3() * -1);
        rtarget.localPosition = new Vector3(-1 * (get_gaze4() + 0.13f), get_gaze5() - 0.1f, get_gaze6() * -1);
        lefteye.LookAt(ltarget);
        righteye.LookAt(rtarget);

    }

    // закрытие потока
    private void OnApplicationQuit()
    {
        newThread.Abort();
    }
}
