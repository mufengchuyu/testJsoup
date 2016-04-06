package testJSoup;

import java.io.File;
import java.io.IOException;

import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;


public class testJSoup {

	public static void main(String[] args) throws IOException {
		//解析HTML文档
//		String html = "<html><head><title>First parse</title></head>"
//				+ "<body><p>Parsed HTML into a doc.</p></body></html>";
//		Document doc = Jsoup.parse(html);
//		System.out.println("" + doc);

		//解析一个本地uri
//		File input = new File("C:/Users/acer/Downloads/index.html");
//		Document doc;
//		try {
//			doc = Jsoup.parse(input, "UTF-8");
//			System.out.println("" + doc);
//		} catch (IOException e) {
//			// TODO Auto-generated catch block
//			e.printStackTrace();
//		}

		//解析body片断
//		File input = new File("C:/Users/acer/Downloads/index.html");
//		Document doc;
//		doc = Jsoup.parse(input, "UTF-8");
//		doc = Jsoup.parseBodyFragment("" + doc);
//		Element body = doc.body();
//		//Elements body = doc.getElementsByTag("body");//这个方法与doc.body();相同
//		System.out.println("" + body);

		//解析一个网络url
//		try {
//			Document doc;
//			doc = Jsoup.connect("https://www.baidu.com/").get();
//			String title = doc.title();
//			System.out.println("" + doc);
//		} catch (IOException e1) {
//			// TODO Auto-generated catch block
//			e1.printStackTrace();
//		}
		
		//Connection 接口还提供一个方法链来解决特殊请求
//		try {
//			//这个方法只支持Web URLs (http和https 协议)
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
		
		//查找元素
		Document doc = Jsoup.connect("http://image.baidu.com/").get();
		Elements links = doc.select("img[src]");		//查找带有src属性的a元素
		System.out.println("" + links);
		int i = 0;
		//获取元素的属性
		for (Element link : links) {
			String linkSrc = link.attr("src");
			System.out.println("" + i + ":" + linkSrc + ":");
			DownloadImage downloadImage = new DownloadImage(linkSrc, "" + i + ".png");
			downloadImage.StartDownload();
			i++;
		}
//		Elements pngs = doc.select("img[src$=.png]");	//查找扩展名为.png的图片
//		System.out.println(""+ "\n\n\n" + pngs);

	}

//Jsoup Selector
//http://jsoup.org/apidocs/org/jsoup/select/Selector.html
}
