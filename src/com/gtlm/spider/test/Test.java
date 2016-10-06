package com.gtlm.spider.test;

import java.util.List;

import com.gtlm.spider.bean.CommonException;
import com.gtlm.spider.bean.News;
import com.gtlm.spider.bean.NewsDto;
import com.gtlm.spider.bean.NewsItem;
import com.gtlm.spider.biz.NewsItemBiz;
import com.gtlm.spider.csdn.Constaint;

/**
 * 测试 使用单元测试
 * 
 * @author acer
 * 
 */
public class Test {

	@org.junit.Test
	public void test02() {
		NewsItemBiz biz = new NewsItemBiz();
		try {
			NewsDto newsDto = biz
					.getNews("http://www.csdn.net/article/2014-04-17/2819363-all-about-ddos");
//			NewsDto newsDto = biz
//					.getNews("http://www.csdn.net/article/2014-04-14/2819277");

			List<News> newses = newsDto.getNewses();
			for (News news : newses) {
				System.out.println(news);

			}

			System.out.println("-----11111-----");
			System.out.println(newsDto.getNextPageUrl());
		} catch (CommonException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	@org.junit.Test
	public void test01() {
		NewsItemBiz biz = new NewsItemBiz();
		int currentPage = 1;
		try {
			// 业界
			List<NewsItem> newsItems = biz.getNewsItems(
					Constaint.NEWS_TYPE_YEJIE, currentPage);
			System.out.println("\n\n\n\n\n\n1" + " Constaint.NEWS_TYPE_YEJIE:"
					+ Constaint.NEWS_TYPE_YEJIE);
			for (NewsItem item : newsItems) {

				System.out.println(item);
			}

			System.out.println("--------------------------");

			// 程序员杂志
			newsItems = biz.getNewsItems(Constaint.NEWS_TYPE_CHENGXUYUAN,
					currentPage);
			System.out.println("\n\n\n\n\n\n4"
					+ " Constaint.NEWS_TYPE_CHENGXUYUAN:"
					+ Constaint.NEWS_TYPE_CHENGXUYUAN);
			for (NewsItem item : newsItems) {

				System.out.println(item);
			}

			System.out.println("--------------------------");

			// 研发
			newsItems = biz
					.getNewsItems(Constaint.NEWS_TYPE_YANFA, currentPage);
			System.out.println("\n\n\n\n\n\n3" + " Constaint.NEWS_TYPE_YANFA:"
					+ Constaint.NEWS_TYPE_YANFA);
			for (NewsItem item : newsItems) {

				System.out.println(item);
			}

			System.out.println("--------------------------");

			// 移动
			newsItems = biz.getNewsItems(Constaint.NEWS_TYPE_YIDONG,
					currentPage);
			System.out.println("\n\n\n\n\n\n2" + " Constaint.NEWS_TYPE_YIDONG:"
					+ Constaint.NEWS_TYPE_YIDONG);
			for (NewsItem item : newsItems) {

				System.out.println(item);
			}

			System.out.println("--------------------------");

			// 云计算
			newsItems = biz.getNewsItems(Constaint.NEWS_TYPE_YUNJISUAN,
					currentPage);
			System.out.println("\n\n\n\n\n\n5"
					+ " Constaint.NEWS_TYPE_YUNJISUAN:"
					+ Constaint.NEWS_TYPE_YUNJISUAN);
			for (NewsItem item : newsItems) {

				System.out.println(item);
			}

			System.out.println("--------------------------");
		} catch (CommonException e) {
			e.printStackTrace();
		}
	}
}
