let socket = io('http://localhost:3000');


const updateData = (field, data) => {
    const fieldDOM = document.getElementById(field);
    let splitInnerText = fieldDOM.innerText.split(' ')
    splitInnerText[0] = data ? data : 'null';
    fieldDOM.innerText = splitInnerText.join(" ")
}

const callAPI = async (url) => {
    const data = await fetch(url).then(async (response) => {
        let { data } = await response.json()
        data = data[0]
        for (let el in data) {
            updateData(el, data[el]);
        }
    })
}

callAPI('http://localhost:3000/download')

socket.on('update', (data) => {
    console.log(data);
    for (let el in data) {
        updateData(el, data[el]);
    }
})