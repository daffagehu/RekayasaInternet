const express = require('express');
const { Pool } = require('pg');
const bodyParser = require('body-parser');
const http = require('http');
const path = require('path');

const app = express();
const server = http.createServer(app);
const port = 3000;

app.use(bodyParser.json());

// Koneksi ke database PostgreSQL
const pool = new Pool({
  user: 'postgres',
  host: 'localhost',
  database: 'rfidDB',
  password: '153426',
  port: 5432,
});

// Endpoint untuk menerima data RFID
app.post('/rfid', (req, res) => {
  const { rfid, name, npm } = req.body;

  // Cek status terakhir untuk RFID tersebut
  const checkStatusQuery = 'SELECT status FROM rfid WHERE rfid = $1 ORDER BY timestamp DESC LIMIT 1';
  pool.query(checkStatusQuery, [rfid], (err, results) => {
    if (err) throw err;

    let newStatus = 'IN'; // Default status
    if (results.rows.length > 0 && results.rows[0].status === 'IN') {
      newStatus = 'OUT';
    }

    // Menyimpan data baru dengan status yang sesuai
    const insertQuery = 'INSERT INTO rfid (rfid, name, npm, status) VALUES ($1, $2, $3, $4)';
    pool.query(insertQuery, [rfid, name, npm, newStatus], (err, result) => {
      if (err) throw err;
      res.status(200).send('RFID saved successfully');
    });
  });
});

// Endpoint untuk mengembalikan data RFID ke frontend
app.get('/rfid-data', (req, res) => {
  const query = 'SELECT * FROM rfid ORDER BY timestamp DESC';
  pool.query(query, (err, results) => {
    if (err) throw err;
    res.json(results.rows);
  });
});

// Menyajikan file statis di folder 'public'
app.use(express.static(path.join(__dirname, 'public')));

// Inisialisasi server HTTP
server.listen(port, () => {
  console.log(`Server berjalan pada http://localhost:${port}`);
});
