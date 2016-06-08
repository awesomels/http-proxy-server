
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
</head>
<?php 

//连接数据库
$link=mysql_connect("localhost","root","");  
if(!$link) echo "没有连接成功!";  
 
//选择数据库
mysql_select_db("filter", $link);

//插入数据库
mysql_query("SET NAMES 'utf8'");
if($_POST["uword"]){
	$q = "INSERT INTO url VALUES (NULL,'$_POST[uword]');";
	$rs = mysql_query($q, $link);
}else if($_POST["fword"]){
	$q = "INSERT INTO fword VALUES (NULL, '$_POST[fword]');";
	$rs = mysql_query($q, $link);
}else if($_GET["del_uword"]){
	$q = "DELETE FROM url WHERE word='$_GET[del_uword]'";
	$rs = mysql_query($q, $link);
}else if($_GET["del_fword"]){
	$q = "DELETE FROM fword WHERE word='$_GET[del_fword]'";
	$rs = mysql_query($q, $link);
}
mysql_close($link);


//重定向浏览器 
header("Location: SettingPage.php"); 
//确保重定向后，后续代码不会被执行 
exit;
?>

