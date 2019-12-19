- curl -d '{"exhibitID":"test", "devices":[{"deviceID":"device1","signalStrength":-30},{"deviceID":"device2","signalStrength":-80}]}' -H "Content-Type: application/json" -X POST http://localhost:3000/methods/exhibitdevices.upsert
- curl localhost:3000/publications/exhibitdevices

