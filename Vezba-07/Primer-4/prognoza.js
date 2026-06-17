function api_call()
{
	var url = "https://api.openweathermap.org/data/2.5/weather?q=" + document.getElementById("naziv_lokacije").value + "&appid=API_KEY&units=metric";
	console.log(url);
	
	var xhttp = new XMLHttpRequest();
	xhttp.onreadystatechange = function() 
	{
		if (this.readyState == 4 && this.status == 200) 
		{
			console.log(this.responseText);
			var obj = JSON.parse(this.responseText);
			console.log(obj);

			document.getElementById("lokacija").innerHTML = obj["name"] + ", " + obj["sys"]["country"];
			document.getElementById("temperatura").innerHTML = obj["main"]["temp"] + "&#x2103;";
			document.getElementById("vlaznost").innerHTML = obj["main"]["humidity"] + "%";
			document.getElementById("pritisak").innerHTML = obj["main"]["pressure"] + " mBar";
			
			document.getElementById("ikonica").src = "http://openweathermap.org/img/wn/" + obj["weather"][0]["icon"] + "@2x.png";
			document.getElementById("opis_vremena").innerHTML = obj["weather"][0]["description"];
			
			document.getElementById("vremenski_podaci").style.display = "block";
			document.getElementById("unos_lokacije").reset();
		}
	};
	xhttp.open("GET", url, false);
	xhttp.send();
}