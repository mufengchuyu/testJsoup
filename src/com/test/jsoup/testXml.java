package com.test.jsoup;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.Element;
import org.dom4j.io.SAXReader;

/**
 * Url url = new Url("http://localhost:8080/Security/update.xml");
((HttpUrlConnection)url.openConnection()).getInputStream();
 * @author acer
 *
 */
public class testXml {

	public static void main(String[] args) throws Exception {
		System.out.println("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
		URL url = new URL("http://localhost:8080/Security/update.xml");
		HttpURLConnection httpURLConnection = (HttpURLConnection)url.openConnection();//打开一个http链接
		httpURLConnection.setConnectTimeout(15*1000);//设置链接的超时时间为5秒
		httpURLConnection.setRequestMethod("GET");//设置请求的方式
		InputStream is = httpURLConnection.getInputStream();//拿到一个输入流。里面包涵了update.xml的信息
		
	
		{
			ByteArrayOutputStream outStream = new ByteArrayOutputStream();
			byte[] buffer = new byte[1024];
			int len = 0;
			while( (len=is.read(buffer)) != -1 ){
				outStream.write(buffer, 0, len);
			}
			is.close();
			
			byte[] btImg = outStream.toByteArray();
			
			if(null != btImg && btImg.length > 0){
				System.out.println("读取到：" + btImg.length + " 字节");
				String str = "";
				for(int i = 0; i < btImg.length; i++){
					str += (char)btImg[i];
					System.out.print("" + btImg[i]);
				}
				System.out.print("\n\n" + str);
			}else{
				System.out.println("没有从该连接获得内容");
			}
		}

		
	}

}
