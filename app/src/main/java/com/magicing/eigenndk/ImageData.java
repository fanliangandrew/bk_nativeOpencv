package com.magicing.eigenndk;

import android.graphics.Bitmap;

import org.json.JSONException;
import org.json.JSONObject;

/**
 * Created by zasx-fanliang on 2017/8/3.
 */

public class ImageData {

    private int[] img;
    private int width;
    private int height;
    private Object[] name;
    private Object[] id;
    private Object[] type;

    public int getWidth() {
        return width;
    }

    public void setWidth(int width) {
        this.width = width;
    }

    public int getHeight() {
        return height;
    }

    public void setHeight(int height) {
        this.height = height;
    }

    public Object[] getType() {
        return type;
    }

    public void setType(Object[] type) {
        this.type = type;
    }

    public Object[] getId() {
        return id;
    }

    public void setId(Object[] id) {
        this.id = id;
    }

    public Object[] getName() {
        return name;
    }

    public void setName(Object[] name) {
        this.name = name;
    }

    public int[] getImg() {
        return img;
    }

    public void setImg(int[] img) {
        this.img = img;
    }

    public ImageData(int height,int width,int[] image,Object[] nameRec,Object[] idRec,Object[] typeRec){
        System.out.println("Construct ImageData start");
        this.height = height;
        this.width = width;
        this.img = image;
        this.name = nameRec;
        this.id   = idRec;
        this.type = typeRec;

        System.out.println("Construct ImageData finish");

    }

    public void printTest(){
        System.out.println("start print name for example");
        int[] tmp;
        System.out.print("name:\n");
        for(int i=0;i<name.length;i++){
            tmp = (int[]) name[i];
            for(int j=0;j<tmp.length;j++){
                System.out.print(" ..."+tmp[j]);
            }
            System.out.print("\n");
        }
        System.out.print("id:\n");
        for(int i=0;i<id.length;i++){
            tmp = (int[]) id[i];
            for(int j=0;j<tmp.length;j++){
                System.out.print(" ..."+tmp[j]);
            }
                System.out.print("\n");
        }
        System.out.print("type\n");
        for(int i=0;i<type.length;i++){
            tmp = (int[]) type[i];
            for(int j=0;j<tmp.length;j++){
                System.out.print(" ..."+tmp[j]);
            }
                System.out.print("\n");
        }
        System.out.println("image length is :"+this.img.length+"\n");
        String test = objectArrayToString(this.name);
        System.out.print(test);
    }

    // obj  <==> int[m][4]
    // convert it to json string
    // {
    //      0: x,y,width,height,
    //      1: x,y,width,height,
    //      ...
    //      m-1:x,y,width,height
    // }
    public String objectArrayToString(Object[] obj){
        int[] temp;
        JSONObject jsonObj = new JSONObject();
        for(int i=0;i<obj.length;i++){
            temp = (int[]) obj[i];
            JSONObject jsonTmp = new JSONObject();
            for(int j=0;j<temp.length;j++){
                try {
                    jsonTmp.put(String.valueOf(j),temp[j]);
                } catch (JSONException e) {
                    e.printStackTrace();
                }
            }
            try {
                jsonObj.put(String.valueOf(i),jsonTmp);
            } catch (JSONException e) {
                e.printStackTrace();
            }
        }
        String result = jsonObj.toString();
        return result;
    }

    public String objectArrayToString(String tag){
        if(tag == "name"){
            return this.objectArrayToString(this.getName());
        }else if(tag == "id"){
            return this.objectArrayToString(this.getId());
        }else if(tag == "type"){
            return this.objectArrayToString(this.getType());
        }else return "unsupport string tag";
    }

    public Bitmap intArrToBitmap(int[] image,int width,int height){
        Bitmap result = Bitmap.createBitmap(width,height, Bitmap.Config.RGB_565);
        result.setPixels(image, 0, width, 0, 0,width, height);
        return result;
    }

    public Bitmap intArrToBitmap(){
        Bitmap result = Bitmap.createBitmap(width,height, Bitmap.Config.RGB_565);
        result.setPixels(this.getImg(), 0, this.getWidth(), 0, 0,this.getWidth(), this.getHeight());
        return result;
    }

    public Boolean checkImgQuality(){
        Boolean good = false;
        if(this.getName().length >=2 && this.getId().length >= 15 && this.getType().length >= 2){
            good = true;
        }
        return good;
    }
}