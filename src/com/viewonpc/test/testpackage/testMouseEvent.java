package com.viewonpc.test.testpackage;

import java.awt.BorderLayout;
import java.awt.Graphics;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionAdapter;

import javax.swing.JFrame;
import javax.swing.JPanel;

public class testMouseEvent extends JFrame{

	public testMouseEvent() {
		MovableMessagePanel p = new MovableMessagePanel("Welcome to java");
		setLayout(new BorderLayout());
		add(p);

	}

	public static void main(String[] args) {
		testMouseEvent frame = new testMouseEvent();
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
					System.out.println("x: " + x + ", y: " + y);
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
				}
			});
		}

		protected void paintComponent(Graphics g) {
			super.paintComponent(g);
			g.drawString(message, x, y);
		}

	}
}
