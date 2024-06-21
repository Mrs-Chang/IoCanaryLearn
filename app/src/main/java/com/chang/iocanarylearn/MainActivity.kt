package com.chang.iocanarylearn

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.TextView
import com.chang.iocanarylearn.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Example of a call to a native method
        binding.sampleText.text = getTimeFromJNI().toString()
    }

    /**
     * A native method that is implemented by the 'iocanarylearn' native library,
     * which is packaged with this application.
     */
    private external fun stringFromJNI(): String
    private external fun getTimeFromJNI(): Long

    companion object {
        // Used to load the 'iocanarylearn' library on application startup.
        init {
            System.loadLibrary("io-canary")
        }
    }
}