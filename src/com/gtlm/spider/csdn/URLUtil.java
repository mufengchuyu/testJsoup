package com.gtlm.spider.csdn;

public class URLUtil {

	public static final String NEWS_LIST_URL = "http://www.csdn.net/headlines.html";
	public static final String NEWS_LIST_URL_YIDONG = "http://mobile.csdn.net/mobile";
	public static final String NEWS_LIST_URL_YANFA = "http://sd.csdn.net/sd";
	public static final String NEWS_LIST_URL_YUNJISUAN = "http://cloud.csdn.net/cloud";
	public static final String NEWS_LIST_URL_ZAZHI = "http://programmer.csdn.net/programmer";
	public static final String NEWS_LIST_URL_YEJIE = "http://news.csdn.net/news";
//	public static final String NEWS_LIST_URL_YIDONG = "http://mobile.csdn.net/";
//	public static final String NEWS_LIST_URL_YANFA = "http://sd.csdn.net/";
//	public static final String NEWS_LIST_URL_YUNJISUAN = "http://cloud.csdn.net/";
//	public static final String NEWS_LIST_URL_ZAZHI = "http://programmer.csdn.net/";
//	public static final String NEWS_LIST_URL_YEJIE = "http://news.csdn.net/";
	
	public static String generateUrl(int newsType, int currentPage){
		currentPage = currentPage > 0 ? currentPage : 1;
		String urlStr = "";
		switch(newsType){
		case Constaint.NEWS_TYPE_YEJIE:
			urlStr = NEWS_LIST_URL_YEJIE;
			break;
		case Constaint.NEWS_TYPE_YANFA:
			urlStr = NEWS_LIST_URL_YANFA;
			break;
		case Constaint.NEWS_TYPE_CHENGXUYUAN:
			urlStr = NEWS_LIST_URL_ZAZHI;
			break;
		case Constaint.NEWS_TYPE_YUNJISUAN:
			urlStr = NEWS_LIST_URL_YUNJISUAN;
			break;
		default:
			urlStr = NEWS_LIST_URL_YIDONG;
			break;
		}
		
		urlStr = urlStr + "/" + currentPage;
		
		return urlStr;
	}
	
}
