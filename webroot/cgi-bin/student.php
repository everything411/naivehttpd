<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Document</title>
</head>
<body>
    <form action="" method="post">
        <input type="text" name="studentid">
        <input type="submit">
    </form>
    <p>
<?php
$link = mysqli_connect(null, "root", "123456", "student");
if (!$link) {
    echo "Error: Unable to connect to MySQL." . PHP_EOL;
    echo "Debugging errno: " . mysqli_connect_errno() . PHP_EOL;
    echo "Debugging error: " . mysqli_connect_error() . PHP_EOL;
    exit;
}
if (isset($_POST["studentid"])) {
    $query = sprintf("select student_id,name,clazz from student where student_id=\"%s\"", $_POST["studentid"]);
    $result = mysqli_query($link, $query);
    if (mysqli_num_rows($result) != 0) {
        $r = $result->fetch_array();
        echo sprintf("student_id: %s name: %s class: %s", $r["student_id"], $r["name"], $r["clazz"]);
    } else {
        echo "no such student";
    }   
}
mysqli_close($link);
?>
    </p>
</body>
</html>
