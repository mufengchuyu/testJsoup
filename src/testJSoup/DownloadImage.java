package testJSoup;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;

/**
 * 实现功能：
 * 将网络图片保存到本地
 * 
 * 属性：
 * imgUrl--图片的网络路径
 * imgName--图片要保存的文件名
 * imgPath--图片要保存的路径
 * 
 * 注意：imgPath路径下的文件夹必须存在，否则无法保存。
 * 后续会添加创建路径下文件夹的功能，
 * 以及从网络路径中获取文件名的功能。
 * 
 * 有两个重大问题没有解决
 * 1， 如果要使用proxy才能连接怎么办？（proxy）
 * 2， 如果你访问的资源需要身份验证怎么办？(java athenticate )
 * 
 * 另外建议把缓冲区大小改大点8K?
 * 
 * 网速卡会导致连接超时：请确保网络不卡顿，并且浏览的网页是流畅的
 */
public class DownloadImage {
	String imgUrl;
	String imgName;
	static String imgPath;
	
	@SuppressWarnings("static-access")
	public DownloadImage(){
		this.imgUrl = "http://www.baidu.com/img/baidu_sylogo1.gif";
		this.imgName = "a.gif";
		this.imgPath = "F:\\";
	}
	
	@SuppressWarnings("static-access")
	public DownloadImage(String imgUrl, String imgName){
		this.imgUrl = imgUrl;
		this.imgName = imgName;
		//这个路径必须要存在，否则会报错。暂时没有搞创建路径的功能，以后会加上。
		this.imgPath = "F:\\Img\\";
	}
	
	@SuppressWarnings("static-access")
	public DownloadImage(String imgUrl, String imgName, String imgPath){
		this.imgUrl = imgUrl;
		this.imgName = imgName;
		this.imgPath = imgPath;
	}
	
	/**
	 * 开始下载图片
	 * (唯一的对外接口)
	 * @param
	 * @return
	 */
	public void StartDownload(){
		byte[] btImg = getImageFromNetByUrl(imgUrl);
		if(null != btImg && btImg.length > 0){
			System.out.println("读取到：" + btImg.length + " 字节");
			writeImageToDisk(btImg, imgName);
		}else{
			System.out.println("没有从该连接获得内容");
		}
	}
	
	/**
	 * 将图片写入到磁盘
	 * @param img 图片数据流
	 * @param fileName 文件保存时的名称
	 */
	protected static void writeImageToDisk(byte[] img, String fileName){
		try {
			File file = new File("" + imgPath + fileName);
			FileOutputStream fops = new FileOutputStream(file);
			fops.write(img);
			fops.flush();
			fops.close();
			System.out.println("图片已经写入到" + imgPath);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	/**
	 * 根据地址获得数据的字节流
	 * @param strUrl 网络连接地址
	 * @return
	 */
	protected static byte[] getImageFromNetByUrl(String strUrl){
		try {
			URL url = new URL(strUrl);
			HttpURLConnection conn = (HttpURLConnection)url.openConnection();
			conn.setRequestMethod("GET");
			conn.setConnectTimeout(15 * 1000);
			InputStream inStream = conn.getInputStream();//通过输入流获取图片数据
			byte[] btImg = readInputStream(inStream);//得到图片的二进制数据
			
			inStream.close();
			return btImg;
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
	}
	/**
	 * 从输入流中获取数据
	 * @param inStream 输入流
	 * @return
	 * @throws Exception
	 */
	protected static byte[] readInputStream(InputStream inStream) throws Exception{
		ByteArrayOutputStream outStream = new ByteArrayOutputStream();
		byte[] buffer = new byte[1024];
		int len = 0;
		while( (len=inStream.read(buffer)) != -1 ){
			outStream.write(buffer, 0, len);
		}
		outStream.close();
		inStream.close();
		return outStream.toByteArray();
	}
	
//这是一个原型图的地址，以后设计网页或是app可以借鉴
//http://1apzuc.axshare.com/#p=index
}

