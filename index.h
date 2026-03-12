const String html = R"=(
<!DOCTYPE html>
<html lang="pl">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Pułapka na koty</title>
    <style>
        *{
            margin: 0px;
            padding: 0px;
            font-size: x-large;
        }
        body{
            background-color: papayawhip;
        }
        img{
            width: calc(100% - 20px);
            max-width: 640px;
            max-height: 480px;
            border: 10px solid;
            border-radius: 20px;
        }
        #camera, #control{
            text-align: center;
            padding: 10px;
        }
        input[type=button], input[type=range], input[type=submit]{
            width: calc(100% - 20px);
            max-width: 640px;
            height: 50px;
            margin-bottom: 10px;
        }  
    </style>
</head>
<body>
    <div id="camera">
        <img src="/stream">
    </div>
    <br>
    <div id="control">
        <form method="post" action="/">
            <input type="button" name="reset" value="Reset mechanizmu" onclick="send('/reset')"><br>
            <input type="button" name="block" value="Zabezpieczenie" onclick="document.getElementById('close')  .disabled = !document.getElementById('close').disabled"><br>
            <input type="button" name="close" value="Zamknij klatkę" id="close" disabled onclick="send('/close')"><br>
            <label for="light">Oświetlenie</label><br>
            <input type="range" name="light" min="0" max="200" value="0"><br>
            <input type="submit" name="confirm" value="Potwierdź"><br>
        </form>    
    </div>
    
</body>
<script>
    function send(uri){
        var xmlHttp = new XMLHttpRequest();
        xmlHttp.onreadystatechange = function() { 
        if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
            alert(xmlHttp.responseText);
        }
        xmlHttp.open("GET", uri, true); // true for asynchronous 
        xmlHttp.send(null);
    }
</script>
</html>
)=";