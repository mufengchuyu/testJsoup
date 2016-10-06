package com.test.testxml;

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
 * 20160422
 * test xmlParser
 */
/**
 * Url url = new Url("http://localhost:8080/Security/update.xml");
((HttpUrlConnection)url.openConnection()).getInputStream();
 * @author acer
 *
 */
public class testXml {

	public static void main(String[] args) throws Exception {
		URL url = new URL("http://localhost:8080/Security/update.xml");
		HttpURLConnection httpURLConnection = (HttpURLConnection)url.openConnection();//��һ��http����
		httpURLConnection.setConnectTimeout(15*1000);//�������ӵĳ�ʱʱ��Ϊ5��
		httpURLConnection.setRequestMethod("GET");//��������ķ�ʽ
		InputStream is = httpURLConnection.getInputStream();//�õ�һ�������������������update.xml����Ϣ
		

		
		//parserXML
		parserXML(is);
	}

	public static void parserXML(InputStream inputStream){
		Map<String, String> map = new HashMap<String, String>();
		// ��ȡ������
		SAXReader reader = new SAXReader();
		Document document;
		try {
			document = reader.read(inputStream);
			// �õ�xml��Ԫ��
			Element root = document.getRootElement();
			// �õ���Ԫ�ص������ӽڵ�
			List<Element> elementList = root.elements();
			
			// ���������ӽڵ�
			for (Element e : elementList) {
				map.put(e.getName(), e.getText());
				System.out.println(""+e.getName() + "\t:\t" + e.getText());
			}

			// �ͷ���Դ
			try {
				inputStream.close();
			} catch (IOException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
			}
			inputStream = null;
		} catch (DocumentException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}

    }
	

	/**
	 * 
�ļ���ʽ
<?xml version=   ecodeinf=   ?>

<parms name="xxx">
    <param name="a">this is for test</param>
    <param name="b">this is for test</param>
    <param name="c">this is for test</param>
</parms>
	 */
	
	/**
	 * 
/////////////////////////////////////////////////////
SAXReader reader = new SAXReader();
try {
	// ��ȡXML�ļ�
	Document doc = reader.read("NewFile.xml");
	Element root = doc.getRootElement();
	System.out.println(root.getName());
	List<Element> param = root.elements();
	for (Element element : param) {
		if(element.attributeValue("name").equals("a")){
			System.out.println(element.getText());
		}
	}
} catch (DocumentException e) {
	e.printStackTrace();
}
        
/////////////////////////////////////////////////////
//way:
// ��ȡ������ 
SAXReader reader = new SAXReader(); 
Document document = reader.read(inputStream); 
// �õ�xml��Ԫ�� 
Element root = document.getRootElement(); 
// �õ���Ԫ�ص������ӽڵ� 
List<Element> elementList = root.elements(); 

// ���������ӽڵ� 
for (Element e : elementList) 
map.put(e.getName(), e.getText()); 

// �ͷ���Դ 
inputStream.close(); 
inputStream = null; 

�ο�http://blog.csdn.net/lyq8479/article/details/8949088
	 

	


/////////////////////////////////////////////////////////////
//way:
public List<Student> doReadXML(String path) throws Exception {
List<Student> empvoList = new ArrayList<Student>();

File file = new File(path);
//����������
FileInputStream fis = new FileInputStream(file);
//jdom������
SAXBuilder sb = new SAXBuilder();
Document doc= sb.build(fis);
//���XML�ĸ�Ԫ��
Element root = doc.getRootElement();
//��ø�Ԫ���µ�������Ԫ��
List<Element> employees = root.getChildren();
for(int i=0;i<employees.size();i++){
Element employee =employees.get(i);
Student stu= new Student();
String name = employee.getChildText("name");
String sex = employee.getChildText("sex");
String agetemp = employee.getChildText("age");
String home = employee.getChildText("home");
String email = employee.getChildText("email");

stu.setName(name);
stu.setSex(sex);
int age = 0;
if(agetemp.equals("")){
age = 0;
} else {
age = Integer.parseInt(agetemp);
}
stu.setAge(age);
stu.setHome(home);
stu.setEmail(email);
System.out.println(name+"\t"+i);
empvoList.add(stu);
}
return empvoList;
}
	 */

}

