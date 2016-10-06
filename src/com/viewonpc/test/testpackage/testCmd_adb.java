package com.viewonpc.test.testpackage;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class testCmd_adb {

	public static void main(String[] args) {
		try {
			Process process = Runtime.getRuntime().exec("cmd /c adb shell input tap " + 957 + " " + 260);
			//Process process = Runtime.getRuntime().exec("cmd /c dir");
			
			OutputStream outputStream = process.getOutputStream();

			final InputStream inputStream = process.getInputStream();
			

			input_content(inputStream);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		System.out.println("start");
	}
	
	public static void output_cmd(OutputStream outputStream){
		try {
			outputStream.write(new byte[] { 'd', 'i', 'r', '\n' });
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	public static void input_content(InputStream inputStream){
		byte[] cache = new byte[1024];
		try {
			if(inputStream.read(cache) != -1){
				System.out.println(new String(cache));
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
}
