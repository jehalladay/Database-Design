// import {
//     config
// } from 'dotenv';

// import {
//     execturePersonCrudOperations
// } from './person.js'

const config = require('dotenv').config;

const execturePersonCrudOperations = require('./person.js').exectutePersonCrudOperations;

config();

console.log('Starting the application');

console.log(process.env.DB_URI);