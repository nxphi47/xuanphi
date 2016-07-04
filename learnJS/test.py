#!/usr/bin/python

import Tkinter
import tkMessageBox

top = Tkinter.Tk()

def helloCallBack():
	tkMessageBox.showinfo("Hello Python", "Hello phi")

Button = Tkinter.Button(top, text = "Hello, click me please", command = helloCallBack)
Button.pack()

C = Tkinter.Canvas(top, bg="blue", height=250, width=300)
coord = 10,50,240,210
arc = C.create_arc(coord, start=0, extent=150, fill="red")
C.pack()
top.mainloop()