// import { 
//     MongoClient 
// } from 'mongodb';

// import {
//     config
// } from 'dotenv';

const MongoClient = require('mongodb').MongoClient;
const config = require('dotenv').config;

config();

async function findPerson(person, name) {
    return person.find({first:name}).toArray();
}



async function connectToCluster(uri) {
    let mongoClient;

    try {
        mongoClient = new MongoClient(uri);
        console.log("Connecting to the cluster " + uri);
        await mongoClient.connect();
        console.log("Connected to the cluster successfully");
    } catch(error) {
        console.log("Error connecting to the cluster " + uri + ': ' + error);
        process.exit()
    }
}




async function executePersonCrudOperations() {
    const uri = process.env.DB_URI;
    let MongoClient;

    try {
        MongoClient = await connectToCluster(uri);
        const db = MongoClient.db('myapp');
        const people = db.collection('people');

        // console.log(await findPerson(people, 'kim'));
    } catch {
        console.log('Error connecting to the cluster');
    } finally {
        await MongoClient.close();
    }
}

