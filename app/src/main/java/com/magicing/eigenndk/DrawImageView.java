package com.magicing.eigenndk;

/**
 * Created by zasx-fanliang on 2017/7/13.
 */


import android.content.Context;
        import android.graphics.Canvas;
        import android.graphics.Color;
        import android.graphics.Paint;
        import android.graphics.Paint.Style;
        import android.graphics.Rect;
        import android.util.AttributeSet;
        import android.widget.ImageView;

public class DrawImageView extends ImageView{

    public DrawImageView(Context context, AttributeSet attrs) {
        super(context, attrs);
        // TODO Auto-generated constructor stub
    }

    Paint paint = new Paint();
    {
        paint.setAntiAlias(true);
        paint.setColor(Color.GREEN);
        paint.setStyle(Style.STROKE);
        paint.setStrokeWidth(5);//设置线宽
        paint.setAlpha(100);
    };

    //矩形的起点
    private int x;
    private int y;

    public static int staticWidth = 1020;
    public static int staticHeight= 1500;
    protected void setVal(int x,int y){
        this.x = x;
        this.y = y;
    }
    @Override
    protected void onDraw(Canvas canvas) {
        // TODO Auto-generated method stub
        super.onDraw(canvas);
        canvas.drawRect(new Rect(this.x,this.y, this.x+staticWidth, this.y+staticHeight), paint);//绘制矩形
    }


}