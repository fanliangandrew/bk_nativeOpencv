package com.magicing.eigenndk;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.graphics.drawable.BitmapDrawable;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.os.SystemClock;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;
import android.view.TextureView;
import android.widget.ImageView;
import android.widget.TextView;

import java.nio.ByteBuffer;
import java.util.Arrays;

public class MainActivity extends AppCompatActivity {

    private TextureView mTextureView;
    private String mCameraId;
    private CameraDevice mCameraDevice;
    private ImageReader mImageReader;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        NDKUtils ndk = new NDKUtils();

        //Demo code for test openCV
//        Bitmap bitmap = ((BitmapDrawable) getResources().getDrawable(
//                R.mipmap.pic_test)).getBitmap();
//        int w = bitmap.getWidth(), h = bitmap.getHeight();
//        int[] pix = new int[w * h];
//        bitmap.getPixels(pix, 0, w, 0, 0, w, h);
//        int [] resultPixes=ndk.gray(pix,w,h);
//        Bitmap result = Bitmap.createBitmap(w,h, Bitmap.Config.RGB_565);
//        result.setPixels(resultPixes, 0, w, 0, 0,w, h);
//
//        ImageView img = (ImageView)findViewById(R.id.img2);
//        img.setImageBitmap(result);

        // Start sett cameras...
        TextureView textureView = (TextureView) findViewById(R.id.textureView);
        mTextureView = textureView;
        mTextureView.setSurfaceTextureListener(textureListener);
    }

    TextureView.SurfaceTextureListener textureListener = new TextureView.SurfaceTextureListener() {
        @Override
        public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
            //当SurefaceTexture可用的时候，设置相机参数并打开相机
            setupCamera(width, height);
            openCamera();
        }
        @Override
        public boolean onSurfaceTextureDestroyed(SurfaceTexture surface){
            mCameraDevice.close();
            return true;
        }
        @Override
        public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width,
                                                int height) {
            // TODO Auto-generated method stub
        }
        @Override
        public void onSurfaceTextureUpdated(SurfaceTexture surface) {
            // TODO Auto-generated method stub
        }
    };


    private void setupCamera(int width, int height) {
        //获取摄像头的管理者CameraManager
        CameraManager manager = (CameraManager) getSystemService(Context.CAMERA_SERVICE);
        try {
            //遍历所有摄像头
            for (String cameraId : manager.getCameraIdList()) {
                CameraCharacteristics characteristics = manager.getCameraCharacteristics(cameraId);
                //默认打开后置摄像头
                if (characteristics.get(CameraCharacteristics.LENS_FACING) == CameraCharacteristics.LENS_FACING_FRONT)
                    continue;
                //获取StreamConfigurationMap，它是管理摄像头支持的所有输出格式和尺寸
                StreamConfigurationMap map = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
                //根据TextureView的尺寸设置预览尺寸
//                mPreviewSize = getOptimalSize(map.getOutputSizes(SurfaceTexture.class), width, height);
                mCameraId = cameraId;
                break;
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }
    private void openCamera() {
        //获取摄像头的管理者CameraManager
        CameraManager manager = (CameraManager) getSystemService(Context.CAMERA_SERVICE);
        //检查权限
        try {
            if (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
                return;
            }
            //打开相机，第一个参数指示打开哪个摄像头，第二个参数stateCallback为相机的状态回调接口，第三个参数用来确定Callback在哪个线程执行，为null的话就在当前线程执行
            manager.openCamera(mCameraId, stateCallback, null);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private final CameraDevice.StateCallback stateCallback = new CameraDevice.StateCallback() {
        @Override
        public void onOpened(CameraDevice camera) {
            Log.i("device","start previewing");
            mCameraDevice = camera;
            //开启预览
            startPreview();
        }
        @Override
        public void onDisconnected(CameraDevice cameraDevice) {
//            closeCameraDevice();
            mCameraDevice.close();
            Log.d("device","disconnected");
        }

        @Override
        public void onError(CameraDevice cameraDevice, int i) {
            Log.w("device","camera start error");
//            Toast.makeText(PusherSurface.this, "摄像头开启失败", Toast.LENGTH_SHORT).show();
        }
    };

    private void startPreview() {
        final CaptureRequest.Builder mCaptureRequestBuilder;

        SurfaceTexture mSurfaceTexture = mTextureView.getSurfaceTexture();
        //设置TextureView的缓冲区大小
        mSurfaceTexture.setDefaultBufferSize(1080, 1920);
        //获取Surface显示预览数据
        Surface mSurface = new Surface(mSurfaceTexture);

        setupImageReader();
        Surface imageRenderSurface = mImageReader.getSurface();

        try {
            //创建CaptureRequestBuilder，TEMPLATE_PREVIEW比表示预览请求
            mCaptureRequestBuilder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            //设置Surface作为预览数据的显示界面
            mCaptureRequestBuilder.addTarget(mSurface);
            mCaptureRequestBuilder.addTarget(imageRenderSurface);
            //创建相机捕获会话，第一个参数是捕获数据的输出Surface列表，第二个参数是CameraCaptureSession的状态回调接口，当它创建好后会回调onConfigured方法，第三个参数用来确定Callback在哪个线程执行，为null的话就在当前线程执行
            mCameraDevice.createCaptureSession(Arrays.asList(mSurface,imageRenderSurface), new CameraCaptureSession.StateCallback() {
                @Override
                public void onConfigured(CameraCaptureSession session) {
                    try {
                        //创建捕获请求
                        CaptureRequest mCaptureRequest = mCaptureRequestBuilder.build();
                        CameraCaptureSession mPreviewSession = session;
                        //设置反复捕获数据的请求，这样预览界面就会一直有数据显示
                        mPreviewSession.setRepeatingRequest(mCaptureRequest, mSessionCaptureCallback, null);
                    } catch (CameraAccessException e) {
                        e.printStackTrace();
                    }
                }

                @Override
                public void onConfigureFailed(CameraCaptureSession session) {

                }
            }, null);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private CameraCaptureSession.CaptureCallback mSessionCaptureCallback = new CameraCaptureSession.CaptureCallback() {
        @Override
        public void onCaptureCompleted(CameraCaptureSession session, CaptureRequest request, TotalCaptureResult result)
        { // Log.d("linc","mSessionCaptureCallback, onCaptureCompleted");
//             mSession = session; checkState(result);
        }
        @Override public void onCaptureProgressed(CameraCaptureSession session, CaptureRequest request, CaptureResult partialResult) {
//                Log.d("linc","mSessionCaptureCallback, onCaptureProgressed"); mSession = session; checkState(partialResult);
        }
//        private void checkState(CaptureResult result) {
//            switch (mState) { case STATE_PREVIEW: // NOTHING break; case STATE_WAITING_CAPTURE: int afState = result.get(CaptureResult.CONTROL_AF_STATE); if (CaptureResult.CONTROL_AF_STATE_FOCUSED_LOCKED == afState || CaptureResult.CONTROL_AF_STATE_NOT_FOCUSED_LOCKED == afState || CaptureResult.CONTROL_AF_STATE_PASSIVE_FOCUSED == afState || CaptureResult.CONTROL_AF_STATE_PASSIVE_UNFOCUSED == afState) { //do something like save picture } break; } }
    };

    //for time control...
//    SimpleDateFormat dfs = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS");
//    private Date time1;
//    private Date time2;
//    private long between = 0;
//
    //    private TextView tmpTextView =(TextView) findViewById(R.id.text_View3);
    private void setupImageReader() {
        //前三个参数分别是需要的尺寸和格式，最后一个参数代表每次最多获取几帧数据，本例的2代表ImageReader中最多可以获取两帧图像流
        mImageReader = ImageReader.newInstance(1080, 1920,
                ImageFormat.JPEG, 2);
        //监听ImageReader的事件，当有图像流数据可用时会回调onImageAvailable方法，它的参数就是预览帧数据，可以对这帧数据进行处理
        mImageReader.setOnImageAvailableListener(new ImageReader.OnImageAvailableListener() {
            private int frames = 0;
            private long initiaTime = SystemClock.elapsedRealtimeNanos();

            @Override
            public void onImageAvailable(ImageReader reader) {
                frames++;
                Image image = null;
                byte[] tmpBytes;
//
//                if((frames % 30) ==0 ){
//                    //我们可以将这帧数据转成字节数组，类似于Camera1的PreviewCallback回调的预览帧数据
//                    Log.e("PreviewListener", "GetPreviewImage");
////                new imageHandle(image);
////                    initiaTime = SystemClock.elapsedRealtimeNanos();
//                }

                try {
                    image = reader.acquireLatestImage();
                    Log.d("PreviewListener", "GetPreviewImage");
                    if (image == null) {
                        return;
                    }
                    if((frames)%30 ==0) {
                        Log.e("PreviewListener", "GetPreviewImage"+image. getFormat()+"  "+ImageFormat. YUV_420_888+"  "+ImageFormat. JPEG);
                        tmpBytes = imageToByteArray(image);

                        NDKUtils ndk = new NDKUtils();

                        Bitmap bitmap = BitmapFactory.decodeByteArray(tmpBytes,0,tmpBytes.length);
                        int w = bitmap.getWidth(), h = bitmap.getHeight();
                        int[] pix = new int[w * h];
                        bitmap.getPixels(pix, 0, w, 0, 0, w, h);
                        int [] resultPixes=ndk.gray(pix,w,h);
                        Bitmap result = Bitmap.createBitmap(w,h, Bitmap.Config.RGB_565);
                        result.setPixels(resultPixes, 0, w, 0, 0,w, h);

                        ImageView img = (ImageView)findViewById(R.id.img2);
                        img.setImageBitmap(result);
                    }
                } finally {
                    if (image != null) {
                        image.close();
                    }
                }
            }
        }, null);
    }

    public class imageHandle implements Runnable {
        private Image mImage;
        public imageHandle(Image image) {
            mImage = image;
        }
        @Override
        public void run() {
            ByteBuffer buffer = mImage.getPlanes()[0].getBuffer();
            byte[] data = new byte[buffer.remaining()];
            buffer.get(data);
        }
    }

    public static byte[] imageToByteArray(Image image) {
        byte[] data = null;
        Image.Plane[] planes = image. getPlanes();
        ByteBuffer buffer = planes[0]. getBuffer();
        data = new byte[buffer.capacity()];
        buffer.get(data);
        return data;
    }
//
//    private static byte[] YUV_420_888toNV21(Image image) {
//        byte[] nv21;
//        ByteBuffer yBuffer = image. getPlanes()[0]. getBuffer();
//        ByteBuffer uBuffer = image. getPlanes()[1]. getBuffer();
//        ByteBuffer vBuffer = image. getPlanes()[2]. getBuffer();
//
//        int ySize = yBuffer. remaining();
//        int uSize = uBuffer. remaining();
//        int vSize = vBuffer. remaining();
//
//        nv21 = new byte[ySize + uSize + vSize];
//
////U and V are swapped
//        yBuffer. get(nv21, 0, ySize);
//        vBuffer. get(nv21, ySize, vSize);
//        uBuffer. get(nv21, ySize + vSize, uSize);
//
//        return nv21;
//    }
}
