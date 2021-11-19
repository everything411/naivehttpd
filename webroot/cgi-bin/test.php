<form action="?a=test" method="post">
    <input type="text" name="test1" value="t1">
    <input type="text" name="test2" value="t2">
    <input type="text" name="test3" value="t3">
    <input type="submit" >
</form>
<?php
    print_r($_GET);
    echo("<br>");
    print_r($_POST);
    echo("<br>");
    print_r($_REQUEST);
    echo("<br>");
    $content = file_get_contents("php://input");
    echo strlen($content);
    echo "<br>";
    echo $content;
    echo "<br>";
    // phpinfo();
?>
