<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<style type="text/css">
div#container{width:800px; margin-left: auto;
margin-right: auto;}
div#header {background-color:#99bbbb;}
div#menu {background-color:#09F; height:50px}
div#URL {background-color:#ffff99;height:500px;width:400px;float:left; border:1px}
div#Fwords {background-color:#EEEEEE;height:500px;width:400px;float:left;border:1px}
h1 {margin-bottom:0;}
h2 {margin-bottom:0;font-size:18px;}
</style>
<title>HTTP Proxy Server</title>
</head>

<body>
<div id="container">

<div id="header">
<h1>HTTP Proxy Server Control Center</h1>
</div>

<div id="menu">
<h3>put button here</h3>
</div>


<div id="URL">
<h2>URL:</h2>
<?php
$link=mysql_connect("localhost","root","");  
if(!$link) echo "没有连接成功!";  
else echo "连接成功!";  
mysql_select_db("filter", $link);  
$q = "SELECT * FROM url";                  
mysql_query("SET NAMES GB2312");          
$rs = mysql_query($q, $link); 
if(!$rs){die("Valid result!");} 
echo "<table border=\"1\">"; 
echo "<tr><td>序号</td><td>屏蔽词</td></tr>"; 
$i = 1;
while($row = mysql_fetch_row($rs)) echo "<tr><td>$i</td><td>$row[1]</td></tr>"; 
echo "</table>"; 
mysql_close($link); 
?>
<form name="input" action="html_form_action.asp" method="get">
<input type="text" name="word" />
<input type="submit" value="添加规则" />
</form>
</div>

<div id="Fwords">
<h2>Fwords:</h2>
<?php
$link=mysql_connect("localhost","root","");  
if(!$link) echo "没有连接成功!";  
else echo "连接成功!";  
mysql_select_db("filter", $link);  
$q = "SELECT * FROM fword";                  
mysql_query("SET NAMES GB2312");          
$rs = mysql_query($q, $link); 
if(!$rs){die("Valid result!");} 
echo "<table border=\"1\">"; 
echo "<tr><td>序号</td><td>屏蔽词</td></tr>"; 
$j = 1;
while($row = mysql_fetch_row($rs)){
 echo "<tr><td>$j</td><td>$row[1]</td></tr>"; 
 $j++;
}
echo "</table>"; 
mysql_close($link); 
?>
<form name="input" action="dothings.php" method="post" >
<input type="text" name="word" />
<input type="submit" value="添加规则" />
</div>

</div>

</body>
</html>
