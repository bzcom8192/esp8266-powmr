#ifndef __HTML_H
#define __HTML_H

#include "Arduino.h"
#include "version.h"

const String style = "body{background-color:black;color:white;font-family:Arial,Helvetica,sans-serif;font-size:14pt}a:visited{color:white}a{text-decoration:none}.info{width:30%;margin:auto;line-height:2rem;text-align:center;border:2px solid darkcyan;border-radius:20px;padding:20px}.update{background-color:darkcyan;padding:12px;border-radius:10px}.update:hover{cursor:pointer}";
const String html = "<html><head><style>" + style + "</style></head><body><div class=\"info\">Hi! I am " + esp + ". " + model + "<br>Version: " + version + "<br>Built: " + built + "<br><br><a class=\"update\" href=\"/update\" class=\"btn1\">UPDATE</a></div></body></html>";

#endif // __HTML_H