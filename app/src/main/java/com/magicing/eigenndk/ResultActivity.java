package com.magicing.eigenndk;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class ResultActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_result);

        Bundle bundle=getIntent().getExtras();
        String str=bundle.getString("showString");

        final TextView mTextView = (TextView) findViewById(R.id.textView5);
        mTextView.setText(str);

        Button reTakeBtn = (Button)findViewById(R.id.button2);
        reTakeBtn.setOnClickListener(new Button.OnClickListener(){
            public void onClick(View v){
                Intent in = new Intent(ResultActivity.this,MainActivity.class);
                startActivity(in);
            }
        });
    }
}
