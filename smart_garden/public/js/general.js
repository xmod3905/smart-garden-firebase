import { initializeApp } from 'https://www.gstatic.com/firebasejs/9.14.0/firebase-app.js'
import { getAnalytics } from 'https://www.gstatic.com/firebasejs/9.14.0/firebase-analytics.js'
import {
  getDatabase,
  ref,
  onValue,
  onChildAdded,
} from 'https://www.gstatic.com/firebasejs/9.14.0/firebase-database.js'

const config = {
  apiKey: "AIzaSyCl0oQYGEespQiaLH27oRTVIc58Rxln03s",
  authDomain: "smart-garden-tekkom.firebaseapp.com",
  databaseURL: 'https://smart-garden-tekkom-default-rtdb.asia-southeast1.firebasedatabase.app/',
  projectId: "smart-garden-tekkom",
  storageBucket: "smart-garden-tekkom.appspot.com",
  messagingSenderId: "416635037179",
  appId: "1:416635037179:web:0f30be292d0ca12bf8175e",
  measurementId: "G-VHNYTRQPZF"
}

const app = initializeApp(config)
const db = getDatabase(app)
const datas = ref(db, '/sensor')
getAnalytics(app)

var temperature = 23.4
const availableUnit = [
  {
    type: 'C',
    value: 'ºC',
    calculate: (val) => val,
  },
  {
    type: 'I',
    value: 'ºF',
    calculate: (val) => val * (9 / 5) + 32,
  },
  {
    type: 'SI',
    value: 'ºK',
    calculate: (val) => val + 273.15,
  },
  {
    type: 'R',
    value: 'ºR',
    calculate: (val) => val * (4 / 5),
  }
]

const defaultUnit = 0
const unitTypes = availableUnit.map((unit) => unit.type)

onValue(
  ref(db, '/pump_status'),(snapshot) => {
    const data = snapshot.val();
    $('#pump').html(data ? 'Hidup ':'Mati');
  } 
);

onChildAdded(
  datas,
  (snapshot) => {
    const data = snapshot.val();
    $('#pressure').html(data.press ?? 'N/A');
    $('#humid').html(data.air_humadity ?? 'N/A');
    $('#soil').html(data.soil_humadity ?? 'N/A');
    $('#daw_point').html(data.daw_point ?? 'N/A');
    $('#heat_index').html(data.head_index ?? 'N/A');
    temperature = parseFloat(data.temperature);
    fillDataValue();
    clock(data.timestamp);
  },
  console.error('Gagal mengambil data')
)

$('#unit-change').on('click', (event) => {
  event.preventDefault()
  fillDataValue(true)
})

const getNextUnit = (unitIdx) => {
  return unitIdx < unitTypes.length ? unitIdx : defaultUnit
}

const fillDataValue = (isChange = false) => {
  let unitType = localStorage.getItem('uniType') ?? availableUnit[defaultUnit].type
  let unitIdx = unitTypes.indexOf(unitType)
  unitIdx = unitIdx === -1 ? defaultUnit : isChange ? (unitIdx += 1) : unitIdx
  updateUnit(getNextUnit(unitIdx))
}

const updateUnit = (unitIdx) => {
  localStorage.setItem('uniType', availableUnit[unitIdx].type)
  const calculateTemperature = availableUnit[unitIdx].calculate(temperature)
  let currentValue = availableUnit[unitIdx].value
  unitIdx = getNextUnit((unitIdx += 1))
  let previousValue = availableUnit[unitIdx].value
  $('#temp-unit').html(currentValue)
  $('#unit-change').html(`Change to ${previousValue}`)

  const value = Math.floor(calculateTemperature)
  const decimal = addZeroPoint(Math.floor((calculateTemperature - value) * 100))
  $('#temp').html(value ?? 'N/A')
  $('#point').html('.' + decimal ?? '00')
}

const addZeroPoint = (int) => (int < 10 ? '0' + int : int)

const generateCSV = (data, isHumi) => {
  const csv = isHumi ? ['Time,Kelembapan Tanah, Kelembapan Udara'] : ['Time,Temperature,Daw Point,Heat Index'];
  var i;
  data.forEach((item) => {
    i++;
    const value = item.val()
    
    if(isHumi){
      csv.push(`${value.timestamp},${value.soil_humadity},${value.air_humadity}`)
    }else{
      csv.push(`${value.timestamp},${value.temperature},${value.daw_point},${value.head_index}`);
    }
  })
  return csv.join(`\n`)
}

const newDate = (unix) => new Date(parseInt(unix) * 1000)

const changeClockFormat = (unix) => {
  const time = newDate(unix)
  const year = time.getFullYear()
  const month = addZeroPoint(time.getMonth())
  const date = addZeroPoint(time.getDate())
  const hour = addZeroPoint(time.getHours())
  const minute = addZeroPoint(time.getMinutes())
  const second = addZeroPoint(time.getSeconds())
  return `${year}-${month}-${date} ${hour}:${minute}:${second}`
}

onValue(datas, (snapshot) => {
  new Dygraph(document.getElementById('chart'), generateCSV(snapshot), {
    title: 'Temperature History',
    ylabel: 'Temperature (C)',
  })
  new Dygraph(document.getElementById('humChart'), generateCSV(snapshot, true), {
    title: 'Humidity History',
    ylabel: '%',
  })
})

const clock = (unix) => {
  const time = newDate(unix)
  const timeInHours = parseInt(
    time.toLocaleString('id-ID', {
      hour: 'numeric',
      timeZone: 'Asia/Jakarta',
    })
  )
  const dayClassName = ['day', 'night']
  // const isNight = timeInHours >= 18 || timeInHours < 6 ? 1 : 0
  const isNight = 0;
  $('#type')
    .removeClass(dayClassName[isNight === 0 ? 1 : 0])
    .addClass(dayClassName[isNight])
  $('#time').html(
    time.toLocaleString('en-GB', {
      timeZone: 'UTC',
      timeStyle: 'long',
      dateStyle: 'long',
    })
  )
  $('#local-time').html(
    time.toLocaleString('id-ID', {
      timeZone: 'Asia/Jakarta',
      timeStyle: 'long',
      dateStyle: 'long',
    })
  )
}
