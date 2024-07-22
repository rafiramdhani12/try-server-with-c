document.getElementById("fetchDataBtn").addEventListener("click", () => {
  fetch("http://localhost:8000/data")
    .then((response) => response.text())
    .then((data) => {
      document.getElementById("serverResponse").innerText = data;
    })
    .catch((error) => {
      console.error("Error fetching data:", error);
    });
});
