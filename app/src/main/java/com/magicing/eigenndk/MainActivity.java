package com.magicing.eigenndk;

import android.Manifest;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.ImageFormat;
import android.graphics.Matrix;
import android.graphics.Point;
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
import android.util.Base64;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.Size;
import android.view.View;
import android.view.Surface;
import android.view.TextureView;
import android.view.Window;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.TextView;

import com.android.volley.Request;
import com.android.volley.Response;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.JsonObjectRequest;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.ByteArrayOutputStream;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.concurrent.LinkedBlockingQueue;


public class MainActivity extends AppCompatActivity {

    private AutoFitTextureView mTextureView;
    private String mCameraId;
    private CameraDevice mCameraDevice;
    private ImageReader mImageReader;

    private ProgressDialog progressDialog;

    private NDKUtils ndk = new NDKUtils();

    final RequestController mReqController = RequestController.getInstance(this);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        requestWindowFeature(Window.FEATURE_NO_TITLE);

        /*set it to be full screen*/
//        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
//                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setContentView(R.layout.activity_main);


        //Demo code for test openCV
        Bitmap bitmap = ((BitmapDrawable) getResources().getDrawable(
                R.mipmap.t0)).getBitmap();
        int w = bitmap.getWidth(), h = bitmap.getHeight();
        int[] pix = new int[w * h];
        bitmap.getPixels(pix, 0, w, 0, 0, w, h);

//        int [] resultPixes=ndk.detectDriving(pix,w,h);
//        Object [] resultPixes=ndk.detectDriving(pix,w,h);
//        System.out.println("  result Pixes:  "+resultPixes);
        final ImageData imageData = ndk.detectDriving(pix,w,h);
        imageData.printTest();
        if(imageData.checkImgQuality()){
                mReqController.addToRequestQueue(getJsonReq(imageData));
        }


//        code for testing android front-page demo picture
//        Bitmap result = Bitmap.createBitmap(w,h, Bitmap.Config.RGB_565);
//        result.setPixels(resultPixes, 0, w, 0, 0,w, h);

        //开辟线程调用
//        mframeQueue = new LinkedBlockingQueue<>();
//        mprocessWithQueue = new ProcessWithQueue(mframeQueue);
//
//
//        int picSize = resultPixes.length-2;
//        int pHeight = resultPixes[picSize],pWidth = resultPixes[picSize+1];
//        int [] tempPixes = new int[picSize];
//        System.arraycopy(resultPixes,0,tempPixes,0,picSize);
//
//        Bitmap result = Bitmap.createBitmap(pWidth,pHeight, Bitmap.Config.RGB_565);
//        result.setPixels(tempPixes, 0, pWidth, 0, 0,pWidth, pHeight);
//
//        ImageView img = (ImageView)findViewById(R.id.imageView);
//        img.setImageBitmap(result);

//        int resultString=ndk.testString(pix,w,h);
//        TextView txtV = (TextView)findViewById(R.id.textView);
//        txtV.setText(String.valueOf(resultString));

//        ImageData imgData=ndk.preDetect(pix,w,h);
//        imgData.printTest();

        //set surface view;

//        msurfaceView = (SurfaceView) findViewById(R.id.surfaceView);

        // Start setting camera...
        AutoFitTextureView textureView = (AutoFitTextureView) findViewById(R.id.textureView);
        mTextureView = textureView;
        mTextureView.setSurfaceTextureListener(textureListener);

        //......
        //draw the green rectangle on the texture View;
        //concentrate on the interest area .
        //......
        mTextureView.post(new Runnable() {
            @Override
            public void run() {
                int tvWidth = mTextureView.getWidth(),tvHeight=mTextureView.getHeight();
                DrawImageView mDrawIV = (DrawImageView)findViewById(R.id.drawIV);
                mDrawIV.bringToFront();
                //ready for bigger
//                pointX = tvWidth/2-510;
//                pointY = tvHeight/2-750;

                //for model algorithm test
                pointX = 220;
                pointY = 300;

                mDrawIV.setVal(pointX,pointY);
                mDrawIV.onDraw(new Canvas());
            }
        });


        Button uploadBtn = (Button)findViewById(R.id.button);
        uploadBtn.setOnClickListener(new Button.OnClickListener(){
            public void onClick(View v){
//                progressDialog = ProgressDialog.show(MainActivity.this, "Loading...", "Please wait...", true, false);
//              Request Server
                mReqController.addToRequestQueue(getJsonReq(imageData));            }
        });

    }

    //vars request to server...
    private String user = "testing";
    private Bitmap mBitmap = null;

    //adjust the screen ratio
    private Size imageDimension;
    private int DSI_height;
    private int DSI_width;

    private void AdjustScreen(CameraCharacteristics characteristics){
        DisplayMetrics displayMetrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
        DSI_height = displayMetrics.heightPixels;
        DSI_width  = displayMetrics.widthPixels;

        StreamConfigurationMap streamConfigurationMap = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
        Size[] sizes = streamConfigurationMap.getOutputSizes(SurfaceTexture.class);
        Log.d("size","testing");
        imageDimension = sizes[0];
        setAspectRatioTextureView(imageDimension.getHeight(),imageDimension.getWidth());
    }

    private void setAspectRatioTextureView(int ResolutionWidth,int ResolutionHeight){
        if(ResolutionWidth>ResolutionHeight){
            int newWidth = DSI_width;
            int newHeight = ((DSI_width * ResolutionWidth)/ResolutionHeight);
            updateTextureViewSize(newWidth,newHeight);
        }else{
            int newWidth = DSI_width;
            int newHeight = ((DSI_width*ResolutionHeight)/ResolutionWidth);
            updateTextureViewSize(newWidth,newHeight);
        }
    }

    private void updateTextureViewSize(int viewWidth, int viewHeight) {
        Log.d("adjust the screen","TextureView Width :  "+viewWidth+"    TextureView Height :  "+viewHeight);
        mTextureView.setLayoutParams(new FrameLayout.LayoutParams(viewWidth,viewHeight));
    }


    //
    private int pointX;
    private int pointY;

    // for new threads to handle more complicated algorithm
    private ProcessWithQueue mprocessWithQueue;
    private LinkedBlockingQueue<byte []> mframeQueue;

    public class ProcessWithQueue extends Thread {
        private static final String TAG = "Queue processing ...";
        private LinkedBlockingQueue<byte[]> mQueue;
        private String str;
        public TextView txtV = (TextView)findViewById(R.id.textView5);

        public ProcessWithQueue(LinkedBlockingQueue<byte[]> frameQueue) {
            mQueue = frameQueue;
            start();
        }

        @Override
        public void run() {
            while (true) {
                byte[] frameData = null;
                try {
                    Log.i(TAG,"taking pictures...");

                    frameData = mQueue.take();

                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                if(frameData !=  null){
                    processFrame(frameData);
                }
            }
        }

        private void processFrame(byte[] tmpBytes) {
            Log.i(TAG, "handling one frame");
            final String textViewString ;


//            Bitmap bitmap = BitmapFactory.decodeByteArray(tmpBytes,0,tmpBytes.length);
//            Matrix matrix = new Matrix();
//            matrix.postRotate((float)90.0);
//            Bitmap rotaBitmap = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth() , bitmap.getHeight() , matrix, false);
////                        Bitmap rotaBitmap = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth() , bitmap.getHeight());
//
//            Bitmap bitmap1 = Bitmap.createBitmap(rotaBitmap,0,0,DrawImageView.staticWidth , DrawImageView.staticHeight);
////                        Bitmap bitmap1 = rotaBitmap;
//
//            int w = bitmap1.getWidth(), h = bitmap1.getHeight();
//            int[] pix = new int[w * h];
//            bitmap1.getPixels(pix, 0, w, 0, 0, w, h);
//            NDKUtils mNdk = new NDKUtils();
//            int [] resultPixes=mNdk.detectDriving(pix,w,h);
//
//            int picSize = resultPixes.length-2;
//            int pHeight = resultPixes[picSize],pWidth = resultPixes[picSize+1];
//            int [] tempPixes = new int[picSize];
//            System.arraycopy(resultPixes,0,tempPixes,0,picSize);
//
//            Bitmap result = Bitmap.createBitmap(pWidth,pHeight, Bitmap.Config.RGB_565);
//            result.setPixels(tempPixes, 0, pWidth, 0, 0,pWidth, pHeight);
//
//            //set the string values
////                        TextView txtV = (TextView)findViewById(R.id.textView);
////                        txtV.setText(String.valueOf(picSize)+"  "+String.valueOf(pHeight)+"  "+String.valueOf(pWidth)+"  "+String.valueOf(tempPixes[0]));
//            textViewString = String.valueOf(picSize)+"  "+String.valueOf(pHeight)+"  "+String.valueOf(pWidth)+"  "+String.valueOf(tempPixes[0]);
////                        Bitmap result = Bitmap.createBitmap(w,h, Bitmap.Config.RGB_565);
////                        result.setPixels(resultPixes, 0, w, 0, 0,w, h);
//
////            ImageView img = (ImageView)findViewById(R.id.imageView);
////            img.setImageBitmap(result);
//
//            txtV.post(new Runnable() {
//                @Override
//                public void run() {
//                    txtV.setText(textViewString);
//                }
//            });

        }
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

    private Size mPreviewSize;
    private int mSensorOrientation;
    /**
     * Max preview width that is guaranteed by Camera2 API
     */
    private static final int MAX_PREVIEW_WIDTH = 1920;

    /**
     * Max preview height that is guaranteed by Camera2 API
     */
    private static final int MAX_PREVIEW_HEIGHT = 1080;
    static class CompareSizesByArea implements Comparator<Size> {

        @Override
        public int compare(Size lhs, Size rhs) {
            // We cast here to ensure the multiplications won't overflow
            return Long.signum((long) lhs.getWidth() * lhs.getHeight() -
                    (long) rhs.getWidth() * rhs.getHeight());
        }

    }
    private static Size chooseOptimalSize(Size[] choices, int textureViewWidth,
                                          int textureViewHeight, int maxWidth, int maxHeight, Size aspectRatio) {

        // Collect the supported resolutions that are at least as big as the preview Surface
        List<Size> bigEnough = new ArrayList<>();
        // Collect the supported resolutions that are smaller than the preview Surface
        List<Size> notBigEnough = new ArrayList<>();
        int w = aspectRatio.getWidth();
        int h = aspectRatio.getHeight();
        for (Size option : choices) {
            if (option.getWidth() <= maxWidth && option.getHeight() <= maxHeight &&
                    option.getHeight() == option.getWidth() * h / w) {
                if (option.getWidth() >= textureViewWidth &&
                        option.getHeight() >= textureViewHeight) {
                    bigEnough.add(option);
                } else {
                    notBigEnough.add(option);
                }
            }
        }

        // Pick the smallest of those big enough. If there is no one big enough, pick the
        // largest of those not big enough.
        if (bigEnough.size() > 0) {
            return Collections.min(bigEnough, new CompareSizesByArea());
        } else if (notBigEnough.size() > 0) {
            return Collections.max(notBigEnough, new CompareSizesByArea());
        } else {
            Log.e("Camera preview", "Couldn't find any suitable preview size");
            return choices[0];
        }
    }
    private Size largest ;

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
                if(map == null ){
                    continue;
                }
                //根据TextureView的尺寸设置预览尺寸
//                mPreviewSize = getOptimalSize(map.getOutputSizes(SurfaceTexture.class), width, height);
                largest = Collections.max(Arrays.asList(map.getOutputSizes(ImageFormat.JPEG)),new CompareSizesByArea());

                int displayRotation = getWindowManager().getDefaultDisplay().getRotation();
                mSensorOrientation = characteristics.get(CameraCharacteristics.SENSOR_ORIENTATION);
                boolean swappedDimension = false;
                switch (displayRotation){
                    case Surface.ROTATION_0:
                    case Surface.ROTATION_180:
                        if(mSensorOrientation == 90 || mSensorOrientation == 270){
                            swappedDimension = true;
                        }
                        break;
                    case Surface.ROTATION_90:
                    case Surface.ROTATION_270:
                        if(mSensorOrientation == 0 || mSensorOrientation == 180){
                            swappedDimension = true;
                        }
                        break;
                    default:
                        Log.e("Camera ", "Display rotation is invalid: " + displayRotation);
                }
                Point displaySize = new Point();
                getWindowManager().getDefaultDisplay().getSize(displaySize);
                int rotatedPreviewWidth = width;
                int rotatedPreviewHeight = height;
                int maxPreviewWidth = displaySize.x;
                int maxPreviewHeight = displaySize.y;

                if (swappedDimension) {
                    rotatedPreviewWidth = height;
                    rotatedPreviewHeight = width;
                    maxPreviewWidth = displaySize.y;
                    maxPreviewHeight = displaySize.x;
                }

                if (maxPreviewWidth > MAX_PREVIEW_WIDTH) {
                    maxPreviewWidth = MAX_PREVIEW_WIDTH;
                }

                if (maxPreviewHeight > MAX_PREVIEW_HEIGHT) {
                    maxPreviewHeight = MAX_PREVIEW_HEIGHT;
                }
                mPreviewSize = chooseOptimalSize(map.getOutputSizes(SurfaceTexture.class),
                        rotatedPreviewWidth, rotatedPreviewHeight, maxPreviewWidth,
                        maxPreviewHeight, largest);
                int orientation = getResources().getConfiguration().orientation;
                if(orientation == Configuration.ORIENTATION_LANDSCAPE){
                    mTextureView.setAspectRatio(mPreviewSize.getWidth(),mPreviewSize.getHeight());
                }else {
                    mTextureView.setAspectRatio(mPreviewSize.getHeight(),mPreviewSize.getWidth());
                }
//                AdjustScreen(characteristics);
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

            //check the camera picture and preview size


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
        mSurfaceTexture.setDefaultBufferSize(1440, 2280);
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
                        //设置参数
//                        mCaptureRequestBuilder.set();
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

    private void setupImageReader() {
        //前三个参数分别是需要的尺寸和格式，最后一个参数代表每次最多获取几帧数据，本例的2代表ImageReader中最多可以获取两帧图像流
        mImageReader = ImageReader.newInstance(largest.getWidth(), largest.getHeight(),
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

                try {
                    image = reader.acquireLatestImage();
                    if (image == null) {
                        return;
                    }
                    if((frames)% 30 ==0) {
//                        Log.e("PreviewListener", "GetPreviewImage");
                        tmpBytes = imageToByteArray(image);
                        Log.d("PreviewListener", "GetPreviewImage image Reader's length :"+String.valueOf(tmpBytes.length));

//                        try{
//                            mframeQueue.put(tmpBytes);
//                        }catch (InterruptedException  e){
//                            e.printStackTrace();
//                        }

                        NDKUtils ndk = new NDKUtils();

                        Bitmap bitmap = BitmapFactory.decodeByteArray(tmpBytes,0,tmpBytes.length);
//                        Matrix matrix = new Matrix();
//                        matrix.postRotate((float)90.0);
//                        Bitmap rotaBitmap = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth() , bitmap.getHeight() , matrix, false);
                        Bitmap rotaBitmap = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth() , bitmap.getHeight());

                        Log.d("PreviewListener","width:   "+rotaBitmap.getWidth()+"   height:"+rotaBitmap.getHeight());

                        Bitmap bitmap1 = Bitmap.createBitmap(rotaBitmap,420,300,2560 , 1680);
//                        Bitmap bitmap1 = rotaBitmap;

                        mBitmap = bitmap1;
                        int w = bitmap1.getWidth(), h = bitmap1.getHeight();
                        int[] pix = new int[w * h];
                        bitmap1.getPixels(pix, 0, w, 0, 0, w, h);
//                        int [] resultPixes=ndk.detectDriving(pix,w,h);

                        Bitmap bitmapTest = ((BitmapDrawable) getResources().getDrawable(
                                R.mipmap.t1)).getBitmap();
                        int wTest = bitmapTest.getWidth(), hTest = bitmapTest.getHeight();
                        int[] pixTest = new int[wTest * hTest];
                        bitmapTest.getPixels(pixTest, 0, wTest, 0, 0, wTest, hTest);

                        ImageData imageData = ndk.detectDriving(pixTest,wTest,hTest);
                        imageData.printTest();
                        if(imageData.checkImgQuality()){
                            mReqController.addToRequestQueue(getJsonReq(imageData));
                        }

//                        int picSize = resultPixes.length-2;
//                        int pHeight = resultPixes[picSize],pWidth = resultPixes[picSize+1];
//                        int [] tempPixes = new int[picSize];
//                        System.arraycopy(resultPixes,0,tempPixes,0,picSize);

//                        Bitmap result = Bitmap.createBitmap(pWidth,pHeight, Bitmap.Config.RGB_565);
//                        result.setPixels(tempPixes, 0, pWidth, 0, 0,pWidth, pHeight);
//                        Matrix matrix = new Matrix();
//                        matrix.postRotate((float)90.0);
//                        Bitmap rotaRest = Bitmap.createBitmap(result, 0, 0, result.getWidth() , result.getHeight() , matrix, false);

                        //set the string values
//                        TextView txtV = (TextView)findViewById(R.id.textView);
//                        txtV.setText(String.valueOf(picSize)+"  "+String.valueOf(pHeight)+"  "+String.valueOf(pWidth)+"  "+String.valueOf(tempPixes[0]));

//                        Bitmap result = Bitmap.createBitmap(w,h, Bitmap.Config.RGB_565);
//                        result.setPixels(resultPixes, 0, w, 0, 0,w, h);

//                        ImageView img = (ImageView)findViewById(R.id.imageView);
//                        img.setImageBitmap(rotaRest);
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

    public JsonObjectRequest getJsonReq(ImageData imageData){
        Bitmap bitmap   = imageData.intArrToBitmap();
        String nameRect = imageData.objectArrayToString("name");
        String idRect   = imageData.objectArrayToString("id");
        String typeRect = imageData.objectArrayToString("type");

        String url = "http://192.168.26.125:3246/validate";
        HashMap<String,Object> params=new HashMap<String,Object>();
        params.put("user","testUser");

        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        bitmap.compress(Bitmap.CompressFormat.JPEG,100,baos);
        byte[] tmp = baos.toByteArray();
        System.out.println(tmp);
        String encodedImage = Base64.encodeToString(tmp, Base64.DEFAULT);
        params.put("bitmap",encodedImage);
        System.out.println("mbitmap height: "+bitmap.getHeight());

        params.put("name",nameRect);
        params.put("id",idRect);
        params.put("type",typeRect);

        JsonObjectRequest jsonObjReq = new JsonObjectRequest(Request.Method.POST,url,
                new JSONObject(params),
                new Response.Listener<JSONObject>() {
                    @Override
                    public void onResponse(JSONObject jsonObject) {
                        System.out.println(jsonObject);
                        try {
//                            progressDialog.dismiss();
                            String tmpRes = (String) jsonObject.get("result");
                            JSONObject res = new JSONObject(tmpRes);
                            String type = (String) res.get("type");
                            String name = (String) res.get("name");
                            String id   = (String) res.get("ID");
                            String show = "type :"+type+"\n"+"name :"+name+"\n"+"id  :"+id;
                            Bundle bundle = new Bundle();
                            bundle.putString("showString",show);
                            Intent in = new Intent(MainActivity.this,ResultActivity.class);
                            in.putExtras(bundle);
                            startActivity(in);

                        } catch (JSONException e) {
                            e.printStackTrace();
                        }
                    }
                },new Response.ErrorListener(){
                    @Override
                    public void onErrorResponse(VolleyError volleyError){
                        System.out.println(volleyError);
                    }
                }
        );
        return jsonObjReq;
    }

}
