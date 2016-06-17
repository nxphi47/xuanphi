#!/usr/bin/python

# Import modules for CGI handling 
import cgi, cgitb
import os

# Create instance of FieldStorage 
form = cgi.FieldStorage()

# Get data from fields
fname = form.getvalue('fname')
lname = form.getvalue('lname')

# Get file from fields
fileItem = form['filename']
if fileItem.filename:
	fn = os.path.basename(fileItem.filename)
	open("/home/phi/Desktop/" + fn, 'wb').write(fileItem.file.read())
	mess = "File '" + fn + "' was uploaded successfully"
else:
	mess = "No File uploaded"

"""
print "Content-type:text/html\r\n\r\n"
print "<html>"
print "<head>"
print "<title>Hello - Second CGI Program</title>"
print "</head>"
print "<body>"
print "<h2>Hello %s %s</h2>" % (fname, lname)
print "<p>%s</p>" % mess
print "</body>"
print "</html>"
"""

template = """
Content-type:text/html\r\n\r\n
<html lang="en">
<head>
	<meta charset="UTF-8">
	<title>Form submitted</title>
</head>
<body>
<h1>User Form submission</h1>

<p>First name is: @@fname@@</p>
<p>last name is: @@lname@@</p>
<p>File: @@file@@</p>

</body>
</html>
"""

htmlList = template.split("@@")
for txt in htmlList:
	if txt == "fname":
		txt = fname
	elif txt == "lname":
		txt = lname
	elif txt == "file":
		txt = mess
	else:
		pass

output = ""
for txt in htmlList:
	output += txt

# output
print output