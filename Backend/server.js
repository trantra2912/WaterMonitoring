const express = require('express')
const cors = require("cors");
const { Pwm } = require('./src/models');
const app = express()

const server = require('http').Server(app);
const io = require('socket.io')(server, {
  cors: {
    origins: 'http://localhost:5500/',
    methods: ["GET", "POST"],
  }
});

app.use(express.urlencoded({ extended: true }));
app.use(express.json());
const port = 3000
app.use(cors());
app.get('/', (req, res) => {
  res.send('Hello World!')
})

app.get('/download', async (req, res) => {
  try {
    const data = await Pwm.findOne({ order: [['createdAt', 'DESC']], raw: true, logging: false, attributes: ['Nhietdo', 'NTU', 'TDS', 'DO_baohoa', 'WQI_TDS', 'WQI_NTU'] });
    res.status(200).json({
      data,
      messsage: 'Success'
    })
  } catch (error) {
    res.status(404).json({
      data: null,
      message: error.message
    })
  }
})



app.get('/upload', async (req, res) => {
  try {
    const fieldsAccept = ['DO_baohoa', 'NTU', 'TDS', 'Nhietdo','WQI_NTU'];
    const queryField = Object.keys(req.query);
    const filterField = []
    queryField.forEach((value) => {
      if (fieldsAccept.includes(value)) {
        filterField.push([value, req.query[value]])
      }
    })
    if (filterField.length === 0) throw new Error('No data');
    const newPwm = await Pwm.create(Object.fromEntries(filterField), { logging: false });
    // let socketData = newPwm.dataValues;
    // delete socketData.id;
    // delete socketData.createdAt;
    // delete socketData.updatedAt;
    const data = await Pwm.findOne({ order: [['createdAt', 'DESC']], raw: true, logging: false, attributes: ['Nhietdo', 'NTU', 'TDS', 'DO_baohoa', 'WQI_NTU'] });

    io.emit('update', data);
    res.status(200).json({
      data: newPwm
    })
  } catch (error) {
    res.status(404).json({
      data: null,
      message: error.message
    })
  }
})


server.listen(port, () => {
  console.log(`Example app listening on port ${port}`)
})