package com.viewonpc.test;
import java.awt.*;
import java.awt.event.*;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import javax.swing.*;

public class MouseMessageDome extends JFrame {

	public MouseMessageDome() {
		MovableMessagePanel p = new MovableMessagePanel("Welcome to java");
		setLayout(new BorderLayout());
		add(p);

	}

	public static void main(String[] args) {
		MouseMessageDome frame = new MouseMessageDome();
		frame.setTitle("MoveMessageDemo");
		frame.setSize(1044, 650);
		frame.setLocationRelativeTo(null);
		// frame.setUndecorated(true);//…Ë÷√Œﬁ±ﬂøÚ
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		frame.setVisible(true);
	}

	static class MovableMessagePanel extends JPanel {
		private String message = "Welcome to java";
		private int x = 20;
		private int y = 20;
		private static boolean flag_frist = true;

		public MovableMessagePanel(String s) {
			message = s;
			addMouseMotionListener(new MouseMotionAdapter() {
				public void mouseDragged(MouseEvent e) {
					int new_x = e.getX();
					int new_y = e.getY();
					if(flag_frist){
						x = new_x;
						y = new_y;
					}
					cmd_do_swipe(x, y, new_x, new_y);
					// repaint();
				}
			});
			addMouseListener(new MouseListener() {

				@Override
				public void mouseReleased(MouseEvent e) {
					// TODO Auto-generated method stub

				}

				@Override
				public void mousePressed(MouseEvent e) {
					// TODO Auto-generated method stub

				}

				@Override
				public void mouseExited(MouseEvent e) {
					// TODO Auto-generated method stub

				}

				@Override
				public void mouseEntered(MouseEvent e) {
					// TODO Auto-generated method stub

				}

				@Override
				public void mouseClicked(MouseEvent e) {

					System.out.println("" + e.getButton());
					System.out.println("x: " + e.getX() + ", y: " + e.getY()
							+ " \n");
					cmd_do_tap(e.getX(), e.getY());
				}
			});
		}

		protected void paintComponent(Graphics g) {
			super.paintComponent(g);
			g.drawString(message, x, y);
		}

	}

	//input swipe 855 255 255 255
	public static void cmd_do_swipe(int old_x, int old_y, int new_x, int new_y){
		try {
			Process process = Runtime.getRuntime().exec("cmd /c adb shell input swipe " + old_x + " " + old_y + " " + new_x + " " + new_y);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	//input tap 957 260
	public static void cmd_do_tap(int x, int y) {
		try {

//			Process process = Runtime.getRuntime().exec("cmd.exe");
//			Process process = Runtime.getRuntime().exec("cmd /c echo.bat install");
//			Process process = Runtime.getRuntime().exec("cmd /c dir");
			Process process = Runtime.getRuntime().exec("cmd /c adb shell input tap " + x + " " + y);

			OutputStream outputStream = process.getOutputStream();

			final InputStream inputStream = process.getInputStream();

			//adb shell input tap 957 260
//			String str_cmd = "adb shell";
//			str_cmd += x + y;
//			byte[] adb_cmd;
//			adb_cmd = str_cmd.getBytes();
//			//outputStream.write(new byte[] { 'd', 'i', 'r', '\n' });
//			outputStream.write(adb_cmd);
			
//			new Thread(new Runnable() {
//
//				byte[] cache = new byte[1024];
//
//				public void run() {
//
//					System.out.println("listener started");
//
//					try {
//
//						while (inputStream.read(cache) != -1) {
//
//							System.out.println(new String(cache));
//
//						}
//
//					} catch (IOException e) {
//
//						// e.printStackTrace();
//
//					}
//
//				}
//
//			}).start();

			
			//outputStream
			output_cmd(outputStream);

			Thread.sleep(1000);
			// inputStream.
			input_content(inputStream);

//			int i = process.waitFor();
//
//			System.out.println("i=" + i);

		} catch (Exception e) {

			e.printStackTrace();
			System.out.println("error++++");

		}
	}
	
	public static void output_cmd(OutputStream outputStream){
		try {
			outputStream.write(new byte[] { 'd', 'i', 'r', '\n' });
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	public static void input_content(InputStream inputStream){
		byte[] cache = new byte[1024];
		try {
			if(inputStream.read(cache) != -1){
				System.out.println(new String(cache));
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
}
