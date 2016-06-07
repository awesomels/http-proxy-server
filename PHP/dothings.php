
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
</head>
<?php 
echo $_POST["word"];
$link=mysql_connect("localhost","root","");  
if(!$link) echo "没有连接成功!";  
else echo "连接成功!";  
mysql_select_db("filter", $link);
$q = "INSERT INTO fword VALUES (NULL,'$_POST[word]');";
mysql_query("SET NAMES 'utf8'");          
$rs = mysql_query($q, $link);
//重定向浏览器 
header("Location: SettingPage.php"); 
//确保重定向后，后续代码不会被执行 
exit;
?>

