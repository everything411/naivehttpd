<form method="post">
    <input type="text" name="num1" value="<?php echo(isset($_POST["num1"]) ? $_POST["num1"] : 0); ?>">
    +
    <input type="text" name="num2" value="<?php echo(isset($_POST["num2"]) ? $_POST["num2"] : 0); ?>">
    = ? 
    <input type="submit" value="计算">
</form>
<?php echo(isset($_POST["num1"]) && isset($_POST["num2"]) ? ($_POST["num1"] + $_POST["num2"]) : ""); ?>
