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
	open("/home/nxphi/Desktop/" + fn, 'wb').write(fileItem.file.read())
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
for i in xrange(len(htmlList)):
	if htmlList[i] == "fname":
		htmlList[i] = fname
	elif htmlList[i] == "lname":
		htmlList[i] = lname
	elif htmlList[i] == "file":
		htmlList[i] = mess
	else:
		pass

output = ""
for txt in htmlList:
	output += txt

# output
print "Content-type:text/html\r\n\r\n"

print output
