package testJSoup;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;

/**
 * ʵ�ֹ��ܣ�
 * ������ͼƬ���浽����
 * 
 * ���ԣ�
 * imgUrl--ͼƬ������·��
 * imgName--ͼƬҪ������ļ���
 * imgPath--ͼƬҪ�����·��
 * 
 * ע�⣺imgPath·���µ��ļ��б�����ڣ������޷����档
 * ��������Ӵ���·�����ļ��еĹ��ܣ�
 * �Լ�������·���л�ȡ�ļ����Ĺ��ܡ�
 * 
 * �������ش�����û�н��
 * 1�� ���Ҫʹ��proxy����������ô�죿��proxy��
 * 2�� �������ʵ���Դ��Ҫ�����֤��ô�죿(java athenticate )
 * 
 * ���⽨��ѻ�������С�Ĵ��8K?
 * 
 * ���ٿ��ᵼ�����ӳ�ʱ����ȷ�����粻���٣������������ҳ��������
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
		//���·������Ҫ���ڣ�����ᱨ����ʱû�и㴴��·���Ĺ��ܣ��Ժ����ϡ�
		this.imgPath = "F:\\Img\\";
	}
	
	@SuppressWarnings("static-access")
	public DownloadImage(String imgUrl, String imgName, String imgPath){
		this.imgUrl = imgUrl;
		this.imgName = imgName;
		this.imgPath = imgPath;
	}
	
	/**
	 * ��ʼ����ͼƬ
	 * (Ψһ�Ķ���ӿ�)
	 * @param
	 * @return
	 */
	public void StartDownload(){
		byte[] btImg = getImageFromNetByUrl(imgUrl);
		if(null != btImg && btImg.length > 0){
			System.out.println("��ȡ����" + btImg.length + " �ֽ�");
			writeImageToDisk(btImg, imgName);
		}else{
			System.out.println("û�дӸ����ӻ������");
		}
	}
	
	/**
	 * ��ͼƬд�뵽����
	 * @param img ͼƬ������
	 * @param fileName �ļ�����ʱ������
	 */
	protected static void writeImageToDisk(byte[] img, String fileName){
		try {
			File file = new File("" + imgPath + fileName);
			FileOutputStream fops = new FileOutputStream(file);
			fops.write(img);
			fops.flush();
			fops.close();
			System.out.println("ͼƬ�Ѿ�д�뵽" + imgPath);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	/**
	 * ���ݵ�ַ������ݵ��ֽ���
	 * @param strUrl �������ӵ�ַ
	 * @return
	 */
	protected static byte[] getImageFromNetByUrl(String strUrl){
		try {
			URL url = new URL(strUrl);
			HttpURLConnection conn = (HttpURLConnection)url.openConnection();
			conn.setRequestMethod("GET");
			conn.setConnectTimeout(15 * 1000);
			InputStream inStream = conn.getInputStream();//ͨ����������ȡͼƬ����
			byte[] btImg = readInputStream(inStream);//�õ�ͼƬ�Ķ���������
			
			inStream.close();
			return btImg;
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
	}
	/**
	 * ���������л�ȡ����
	 * @param inStream ������
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
	
//����һ��ԭ��ͼ�ĵ�ַ���Ժ������ҳ����app���Խ��
//http://1apzuc.axshare.com/#p=index
}

