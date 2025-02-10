const path = require('path');
const TOKEN_FILE = path.join(__dirname, 'tokens.json');
const fs = require("fs");
const axios = require("axios");
const config = JSON.parse(fs.readFileSync(path.join(__dirname, "config.json")));

// const config = {
//   tokenUrl: 'YOUR_TOKEN_URL',
//   clientId: 'YOUR_CLIENT_ID',
//   clientSecret: 'YOUR_CLIENT_SECRET'
// };



  // access_token: '5d06b52cc52009000a5ef4d2|f8fb37c03cd86f7998546c4bd5b044b8',
  // refresh_token: '5d06b52cc52009000a5ef4d2|572dea9e8da5492db84ec26e471ba185'


async function refreshaccess_token(refresh_token) {


  config.tokens = await loadTokens();



  try {
    console.log('Using token URL:', config.tokenUrl); // Dodaj logowanie URL
    const response = await axios.post(config.tokenUrl, new URLSearchParams({
      grant_type: 'refresh_token',
      refresh_token: config.tokens.refresh_token,
      client_id: config.clientId,
      client_secret: config.clientSecret
    }));


    console.log('Response from token refresh:', response.data); // Dodaj logowanie odpowiedzi
    const access_token = response.data.access_token;
    const newrefresh_token = response.data.refresh_token;
    saveTokens(access_token, newrefresh_token);
    return { access_token, refresh_token: newrefresh_token };
  } catch (error) {


    // console.log(
    //   'grant_type:', 'refresh_token',
    //   'refresh_token:', config.tokens.refresh_token,
    //   'client_id:', config.clientId,
    //   'client_secret:', config.clientSecret


    // );

    console.error('Unable to refresh access token:', error.response ? error.response.data : error.message);
    return { error: 'Unable to refresh access token' };
  }
}

function saveTokens(access_token, refresh_token) {
  const tokens = { access_token: access_token, refresh_token: refresh_token };
  fs.writeFileSync(TOKEN_FILE, JSON.stringify(tokens));
}

async function loadTokens() {
  const tokens = JSON.parse(fs.readFileSync(TOKEN_FILE));
  if (!tokens.access_token || !tokens.refresh_token) {
    throw new Error('Invalid token file');
  }
  // Odśwież tokeny, jeśli są nieaktualne
  return tokens;
}

module.exports = { refreshaccess_token, saveTokens, loadTokens };