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
		HttpURLConnection httpURLConnection = (HttpURLConnection)url.openConnection();//��һ��http����
		httpURLConnection.setConnectTimeout(15*1000);//�������ӵĳ�ʱʱ��Ϊ5��
		httpURLConnection.setRequestMethod("GET");//��������ķ�ʽ
		InputStream is = httpURLConnection.getInputStream();//�õ�һ�������������������update.xml����Ϣ
		
	
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
				System.out.println("��ȡ����" + btImg.length + " �ֽ�");
				String str = "";
				for(int i = 0; i < btImg.length; i++){
					str += (char)btImg[i];
					System.out.print("" + btImg[i]);
				}
				System.out.print("\n\n" + str);
			}else{
				System.out.println("û�дӸ����ӻ������");
			}
		}

		
	}

}
