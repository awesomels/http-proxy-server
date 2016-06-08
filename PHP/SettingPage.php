<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">

<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<style type="text/css">
div#container{width:800px; margin-left: auto;
margin-right: auto;}
div#header {background-color:#99bbbb;}
div#menu {background-color:#999; height:50px}
div#URL {height:500px;width:350px;float:left; border:1px; margin:0 25px}
div#Fwords {height:500px;width:350px;float:left;border:1px margin:0 25px}
h1 {margin-bottom:0;}
h2 {margin-bottom:0;font-size:18px;}


#customers
  {
  font-family:"Trebuchet MS", Arial, Helvetica, sans-serif;
  width:100%;
  border-collapse:collapse;
  }



#customers th 
  {
  font-size:1.1em;
  text-align:left;
  padding-top:5px;
  padding-bottom:4px;
  background-color:#A7C942;
  color:#ffffff;
  }

#customers tr.alt td 
  {
  color:#000000;
  background-color:#EAF2D3;
  }

</style>
<title>HTTP Proxy Server</title>
</head>

<body>
<div id="container">

<div id="header">
<h1 align="center">内容安全的HTTP代理服务器控制中心</h1>
</div>

<div id="menu">
<h3>put button here</h3>
</div>


<div id="URL">
<h2>URL:</h2>
<?php

$link=mysql_connect("localhost","root","");  
if(!$link) echo "没有连接成功!";  

mysql_select_db("filter", $link);  
$q = "SELECT * FROM url";                  
mysql_query("SET NAMES GB2312");          
$rs = mysql_query($q, $link); 
if(!$rs){die("Valid result!");} 

echo "<table id=\"customers\">"; 
echo "<tr><th>序号</th><th>屏蔽词</th><th>操作</th></tr>"; 
$i = 1;
while($row = mysql_fetch_row($rs)){
	if($i%2)
		echo "<tr class=\"alt\"><td>$i</td><td>$row[1]</td><td><a href='dothings.php?del_uword=$row[1]'>删除</a></td></tr>"; 
	else
		echo "<tr><td>$i</td><td>$row[1]</td><td><a href='dothings.php?del_uword=$row[1]'>删除</a></td></tr>";
	$i++;
}
echo "</table>"; 

mysql_close($link); 
?>

<form name="input" action="dothings.php" method="post" >
<input type="text" name="uword" />
<input type="submit" value="添加规则" />
</form>
</div>

<div id="Fwords">
<h2>Fwords:</h2>
<?php
$link=mysql_connect("localhost","root","");  
if(!$link) echo "没有连接成功!";  
 
mysql_select_db("filter", $link);  
$q = "SELECT * FROM fword";                  
mysql_query("SET NAMES GB2312");          
$rs = mysql_query($q, $link); 
if(!$rs){die("Valid result!");} 
echo "<table id=\"customers\">"; 
echo "<tr><th>序号</th><th>屏蔽词</th><th>操作</th></tr>"; 
$j = 1;
while($row = mysql_fetch_row($rs)){
	if($j%2)
 		echo "<tr class=\"alt\"><td>$j</td><td>$row[1]</td><td><a href='dothings.php?del_fword=$row[1]'>删除</a></td></tr>"; 
	else
		echo "<tr><td>$j</td><td>$row[1]</td><td><a href='dothings.php?del_fword=$row[1]'>删除</a></td></tr>";
 $j++;
}
echo "</table>"; 
mysql_close($link); 
?>
<form name="input" action="dothings.php" method="post" >
<input type="text" name="fword" />
<input type="submit" value="添加规则" />
</div>

</div>
</body>
</html>
