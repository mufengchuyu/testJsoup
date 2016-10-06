package com.gtlm.spider.biz;

import java.util.ArrayList;
import java.util.List;

import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;

import com.gtlm.spider.bean.CommonException;
import com.gtlm.spider.bean.News;
import com.gtlm.spider.bean.News.NewsType;
import com.gtlm.spider.bean.NewsDto;
import com.gtlm.spider.bean.NewsItem;
import com.gtlm.spider.csdn.DataUtil;
import com.gtlm.spider.csdn.URLUtil;

/**
 * 处理NewsItem的业务类
 * 
 * @author acer
 * 
 */
public class NewsItemBiz {

	/**
	 * 业界、移动、云计算
	 * 
	 * @param newsType
	 * @param currentPage
	 * @return
	 * @throws CommonException
	 */
	public List<NewsItem> getNewsItems(int newsType, int currentPage)
			throws CommonException {
		String urlStr = URLUtil.generateUrl(newsType, currentPage);
		String htmlStr = DataUtil.doGet(urlStr);

		List<NewsItem> newsItems = new ArrayList<NewsItem>();
		NewsItem newsItem = null;

		Document doc = Jsoup.parse(htmlStr);
		Elements units = doc.getElementsByClass("unit");
		for (int i = 0; i < units.size(); i++) {
			newsItem = new NewsItem();
			newsItem.setNewsType(newsType);

			Element unit_ele = units.get(i);

			Element h1_ele = unit_ele.getElementsByTag("h1").get(0);
			Element h1_a_ele = h1_ele.child(0);
			String title = DataUtil.ToDBC(h1_a_ele.text());
			String href = h1_a_ele.attr("href");

			newsItem.setLink(href);
			newsItem.setTitle(title);

			Element h4_ele = unit_ele.getElementsByTag("h4").get(0);
			Element ago_ele = h4_ele.getElementsByClass("ago").get(0);
			String date = ago_ele.text();

			newsItem.setDate(date);

			Element dl_ele = unit_ele.getElementsByTag("dl").get(0);// dl
			Element dt_ele = dl_ele.child(0);// dt
			try {
				// maybe have not image
				Element img_ele = dt_ele.child(0);
				String imgLink = img_ele.child(0).attr("src");
				newsItem.setImgLink(imgLink);
			} catch (IndexOutOfBoundsException e) {

			}

			Element content_ele = dl_ele.child(1);// dd
			String content = DataUtil.ToDBC(content_ele.text());
			newsItem.setContent(content);
			newsItems.add(newsItem);
		}

		return newsItems;
	}

	/**
	 * 根据文章的url返回一个NewsDto对象
	 * 
	 * @return
	 * @throws CommonException
	 */
	public NewsDto getNews(String urlStr) throws CommonException {
		NewsDto newsDto = new NewsDto();
		List<News> newses = new ArrayList<News>();
		String htmlStr = DataUtil.doGet(urlStr);
		Document doc = Jsoup.parse(htmlStr);

		// 获得文章中的第一个detail
		Element detailEle = doc.select(".left .detail").get(0);
		// 标题
		Element titleEle = detailEle.select("h1.title").get(0);
		News news = new News();
		news.setTitle(titleEle.text());
		news.setType(NewsType.TITLE);
		newses.add(news);
		// 摘要
		Element summaryEle = detailEle.select("div.summary").get(0);
		news = new News();
		news.setSummary(summaryEle.text());
		newses.add(news);
		// 内容
		Element contentEle = detailEle.select("div.con.news_content").get(0);
		Elements childrenEle = contentEle.children();

		for (Element child : childrenEle) {
			Elements imgEles = child.getElementsByTag("img");
			// 图片
			if (imgEles.size() > 0) {
				for (Element imgEle : imgEles) {
					if (imgEle.attr("src").equals(""))
						continue;
					news = new News();
					news.setImageLink(imgEle.attr("src"));
					newses.add(news);
				}
			}
			// 移除图片
			imgEles.remove();

			if (child.text().equals(""))
				continue;

			news = new News();
			news.setType(NewsType.CONTENT);

			try {
				if (child.children().size() == 1) {
					Element cc = child.child(0);
					if (cc.tagName().equals("b")) {
						news.setType(NewsType.BOLD_TITLE);
					}
				}

			} catch (IndexOutOfBoundsException e) {
				e.printStackTrace();
			}
			news.setContent(child.outerHtml());
			newses.add(news);
		}
		newsDto.setNewses(newses);
		return newsDto;
	}
}
