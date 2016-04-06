package testJSoup;

import java.io.File;
import java.io.IOException;

import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;


public class testJSoup {

	public static void main(String[] args) throws IOException {
		//����HTML�ĵ�
//		String html = "<html><head><title>First parse</title></head>"
//				+ "<body><p>Parsed HTML into a doc.</p></body></html>";
//		Document doc = Jsoup.parse(html);
//		System.out.println("" + doc);

		//����һ������uri
//		File input = new File("C:/Users/acer/Downloads/index.html");
//		Document doc;
//		try {
//			doc = Jsoup.parse(input, "UTF-8");
//			System.out.println("" + doc);
//		} catch (IOException e) {
//			// TODO Auto-generated catch block
//			e.printStackTrace();
//		}

		//����bodyƬ��
//		File input = new File("C:/Users/acer/Downloads/index.html");
//		Document doc;
//		doc = Jsoup.parse(input, "UTF-8");
//		doc = Jsoup.parseBodyFragment("" + doc);
//		Element body = doc.body();
//		//Elements body = doc.getElementsByTag("body");//���������doc.body();��ͬ
//		System.out.println("" + body);

		//����һ������url
//		try {
//			Document doc;
//			doc = Jsoup.connect("https://www.baidu.com/").get();
//			String title = doc.title();
//			System.out.println("" + doc);
//		} catch (IOException e1) {
//			// TODO Auto-generated catch block
//			e1.printStackTrace();
//		}
		
		//Connection �ӿڻ��ṩһ���������������������
//		try {
//			//�������ֻ֧��Web URLs (http��https Э��)
//			Document doc = Jsoup.connect("http://image.baidu.com/")
//					.data("query", "Java")
//					.userAgent("Mozilla")
//					.cookie("auth", "token")
//					.timeout(3000)
//					.post();
//			System.out.println("" + doc);
//		} catch (IOException e) {
//			// TODO Auto-generated catch block
//			e.printStackTrace();
//		}
		
		//����Ԫ��
		Document doc = Jsoup.connect("http://image.baidu.com/").get();
		Elements links = doc.select("img[src]");		//���Ҵ���src���Ե�aԪ��
		System.out.println("" + links);
		int i = 0;
		//��ȡԪ�ص�����
		for (Element link : links) {
			String linkSrc = link.attr("src");
			System.out.println("" + i + ":" + linkSrc + ":");
			DownloadImage downloadImage = new DownloadImage(linkSrc, "" + i + ".png");
			downloadImage.StartDownload();
			i++;
		}
//		Elements pngs = doc.select("img[src$=.png]");	//������չ��Ϊ.png��ͼƬ
//		System.out.println(""+ "\n\n\n" + pngs);

	}

//Jsoup Selector
//http://jsoup.org/apidocs/org/jsoup/select/Selector.html
}
