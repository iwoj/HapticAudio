//
//  ViewController.swift
//  closest-beacon-demo
//
//  Created by Will Dages on 10/11/14.
//  @willdages on Twitter
//

import Foundation
import UIKit
import CoreLocation
import AVFoundation
import CoreNFC
//import Meteor

class ViewController: UIViewController, CLLocationManagerDelegate, AVAudioPlayerDelegate, UITextViewDelegate, NFCNDEFReaderSessionDelegate {
    
    @IBOutlet weak var log:UITextView!
    @IBOutlet weak var modeSwitch:UISwitch!
    
//    let Meteor = METCoreDataDDPClient(serverURL: URL(string: "wss://meteor-ios-todos.meteor.com/websocket")!)
    
    let regionUUID = "00000000-0000-0000-0000-000000000001"
    let regionIdentifier = "Redwoods"
    let region:CLBeaconRegion

    let audioSession = AVAudioSession.sharedInstance()
    var nfcSession:NFCNDEFReaderSession?
    let locationManager = CLLocationManager()
    
    let fadeDuration = 2.0
    
    let noBeaconID = "00000000-0000-0000-0000-000000000000.0.0"
    var previousBeaconID:String
    
    let DUAL_MODE = "dual"
    let PROXIMITY_MODE = "proximity"
    let NFC_MODE = "nfc"
    var currentMode:String
    
    var exhibits = [String: Exhibit]()
    let exhibitSettings: [Dictionary<String, Any>] = [
        [
            "color": UIColor(red: 200/255, green: 200/255, blue: 200/255, alpha: 1),
            "soundPath": Bundle.main.path(forResource: "background", ofType: "mp3")!,
            "beaconID": ["00000000-0000-0000-0000-000000000000", 0, 0],
            "rangeLimit": 0,
            "nfcID": "00:00:00:00:00:00:00"
        ],
        [
            "color": UIColor(red: 84/255, green: 77/255, blue: 160/255, alpha: 1),
            "soundPath": Bundle.main.path(forResource: "habitat", ofType: "mp3")!,
            "beaconID": ["00000000-0000-0000-0000-000000000001", 1, 1],
            "rangeLimit": -55,
            "nfcID": ""
        ],
        [
            "color": UIColor(red: 84/255, green: 77/255, blue: 160/255, alpha: 1),
            "soundPath": Bundle.main.path(forResource: "habitat-intro", ofType: "mp3")!,
            "beaconID": ["", 0, 0],
            "rangeLimit": -58,
            "nfcID": "04:48:A9:7A:B0:57:81"
        ],
        [
            "color": UIColor(red: 220/255, green: 120/255, blue: 120/255, alpha: 1),
            "soundPath": Bundle.main.path(forResource: "walk-in-the-parks", ofType: "mp3")!,
            "beaconID": ["00000000-0000-0000-0000-000000000001", 1, 2],
            "rangeLimit": -58,
            "nfcID": ""
        ],
        [
            "color": UIColor(red: 220/255, green: 120/255, blue: 120/255, alpha: 1),
            "soundPath": Bundle.main.path(forResource: "walk-in-the-parks-intro", ofType: "mp3")!,
            "beaconID": ["", 0, 0],
            "rangeLimit": -55,
            "nfcID": "04:39:A9:7A:B0:57:81"
        ],
        [
            "color": UIColor(red: 64/255, green: 220/255, blue: 32/255, alpha: 1),
            "soundPath": Bundle.main.path(forResource: "restoration", ofType: "mp3")!,
            "beaconID": ["00000000-0000-0000-0000-000000000001", 1, 3],
            "rangeLimit": -60,
            "nfcID": ""
        ],
        [
            "color": UIColor(red: 64/255, green: 220/255, blue: 32/255, alpha: 1),
            "soundPath": Bundle.main.path(forResource: "restoration-intro", ofType: "mp3")!,
            "beaconID": ["", 0, 0],
            "rangeLimit": -63,
            "nfcID": "04:3E:A9:7A:B0:57:81"
        ],
    ]
    
    required init?(coder aDecoder: NSCoder) {
        previousBeaconID = noBeaconID
        region = CLBeaconRegion(proximityUUID: UUID(uuidString: regionUUID)!, identifier: regionIdentifier)
        currentMode = NFC_MODE
        super.init(coder: aDecoder)
    }
    
    @IBAction func doSwitch(sender: UISwitch) {
        if (sender.isOn) {
            setMode(NFC_MODE)
        }
        else {
            setMode(PROXIMITY_MODE)
        }
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        log.delegate = self
        
//        Meteor.connect()
        
        setMode(NFC_MODE)
        
        locationManager.delegate = self
        if (CLLocationManager.authorizationStatus() != CLAuthorizationStatus.authorizedWhenInUse) {
            locationManager.requestWhenInUseAuthorization()
        }
        
//        configureAudioSessionCategory()
//        configureAudioSessionToSpeaker()
        
        initExhibits(settings: exhibitSettings)
        
        // Set to off-exhibit state
        self.view.backgroundColor = exhibits["00:00:00:00:00:00:00"]?.color
        exhibits["00:00:00:00:00:00:00"]?.audioPlayer.play()
        exhibits["00:00:00:00:00:00:00"]?.audioPlayer.setVolume(1, fadeDuration: fadeDuration)
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    func setMode(_ mode:String) {
        switch mode {
        case PROXIMITY_MODE:
            locationManager.startRangingBeacons(in: region)
            nfcSession?.invalidate()
            currentMode = mode
            modeSwitch.setOn(false, animated: true)
        case NFC_MODE:
            locationManager.stopRangingBeacons(in: region)
            nfcSession = NFCNDEFReaderSession(delegate: self, queue: nil, invalidateAfterFirstRead: false)
            nfcSession?.begin()
            currentMode = mode
            modeSwitch.setOn(true, animated: true)
        case DUAL_MODE:
            locationManager.startRangingBeacons(in: region)
            nfcSession = NFCNDEFReaderSession(delegate: self, queue: nil, invalidateAfterFirstRead: false)
            nfcSession?.begin()
            currentMode = mode
        default:
            print("Invalid mode.")
        }
    }
    
    func initExhibits(settings: [Dictionary<String, Any>]) {
        for setting in settings {
            do {
                let exhibit = try Exhibit(color: setting["color"] as! UIColor,
                                          soundPath: setting["soundPath"] as! String,
                                          beaconID: setting["beaconID"] as! [AnyObject],
                                          rangeLimit: setting["rangeLimit"] as! Int,
                                          nfcID: setting["nfcID"] as! String)
                exhibits[exhibit.beaconCombinedID] = exhibit
                exhibits[exhibit.nfcID] = exhibit
            }
            catch {
                print(error)
            }
        }
    }
    
    func locationManager(_ manager: CLLocationManager, didRangeBeacons beacons: [CLBeacon], in region: CLBeaconRegion) {
//        var knownBeacons = beacons
//        var knownBeacons = beacons.filter{ $0.proximity == CLProximity.immediate }
//        var knownBeacons = beacons.filter{ $0.proximity == CLProximity.immediate || $0.proximity == CLProximity.near }
        var knownBeacons = beacons.filter{
            let beaconID = getBeaconID($0)
            return exhibits[beaconID]!.rangeLimit < $0.rssi // Apply range limits
                && $0.rssi != 0 // Filter out glitches
        }
        knownBeacons = knownBeacons.sorted(
            by: {
                let beaconID = getBeaconID($0)
                let rangeLimit = exhibits[beaconID]!.rangeLimit
                return 1-($0.rssi/rangeLimit) > 1-($1.rssi/rangeLimit)
            }
        )
        
        log.text = ""
        for beacon in knownBeacons {
            logBeacon(beacon)
        }
        
        if (knownBeacons.count > 0) {
            let closestBeacon = knownBeacons[0] as CLBeacon
            let closestBeaconID = "\(closestBeacon.proximityUUID.uuidString).\(closestBeacon.major.intValue).\(closestBeacon.minor.intValue)"
            
            if (previousBeaconID != closestBeaconID) {
                playExhibitAudio(closestBeaconID)
            }
        }
        else {
            if (previousBeaconID != noBeaconID) {
                log.text = ""
                playExhibitAudio(noBeaconID)
            }
        }
    }
    
    func playExhibitAudio(_ exhibitID: String) {
        self.view.backgroundColor = exhibits[exhibitID]?.color
        
        exhibits[exhibitID]?.stopTask.cancel()
        exhibits[exhibitID]?.audioPlayer.volume = 0
        exhibits[exhibitID]?.audioPlayer.numberOfLoops = -1
        exhibits[exhibitID]?.audioPlayer.play()
        exhibits[exhibitID]?.audioPlayer.setVolume(1, fadeDuration: fadeDuration)
        
        exhibits[previousBeaconID]?.audioPlayer.setVolume(0, fadeDuration: fadeDuration)
        let x = previousBeaconID
        exhibits[x]?.stopTask = DispatchWorkItem {
            self.exhibits[x]?.audioPlayer.stop()
        }
        DispatchQueue.main.asyncAfter(deadline: DispatchTime.now() + fadeDuration, execute: exhibits[x]!.stopTask)
        
        previousBeaconID = exhibitID
    }
    
    func configureAudioSessionCategory() {
        print("Configuring audio session")
        do {
            try audioSession.setCategory(AVAudioSession.Category.playAndRecord, mode: AVAudioSession.Mode.voiceChat)
//            try audioSession.setCategory(AVAudioSession.Category.playback, mode: AVAudioSession.Mode.voiceChat)
            try audioSession.overrideOutputAudioPort(AVAudioSession.PortOverride.none)
            print("AVAudio Session out options: ", audioSession.currentRoute)
            print("Successfully configured audio session.")
        } catch (let error) {
            print("Error while configuring audio session: \(error)")
        }
    }
    
    func configureAudioSessionToSpeaker(){
        do {
            try audioSession.overrideOutputAudioPort(AVAudioSession.PortOverride.speaker)
            try audioSession.setActive(true)
            print("Successfully configured audio session (SPEAKER-Bottom).", "\nCurrent audio route: ",audioSession.currentRoute.outputs)
        } catch let error as NSError {
            print("#configureAudioSessionToSpeaker Error \(error.localizedDescription)")
        }
    }
    
    func configureAudioSessionToEarSpeaker(){
        
        let audioSession:AVAudioSession = AVAudioSession.sharedInstance()
        do { ///Audio Session: Set on Speaker
            try audioSession.overrideOutputAudioPort(AVAudioSession.PortOverride.none)
            try audioSession.setActive(true)
            
            print("Successfully configured audio session (EAR-Speaker).", "\nCurrent audio route: ",audioSession.currentRoute.outputs)
        }
        catch{
            print("#configureAudioSessionToEarSpeaker Error \(error.localizedDescription)")
        }
    }
    
    func readerSession(_ session: NFCNDEFReaderSession, didDetectNDEFs messages: [NFCNDEFMessage]) {
        DispatchQueue.main.async {
            let nfcID = messages[0].records[0].payload.advanced(by: 3).base64EncodedString().base64Decoded()
            // Process detected NFCNDEFMessage objects.
            self.log.text = ""
            self.log.text += "\(nfcID ?? "")\n"
//            if (currentMode == DUAL_MODE) {
//                locationManager.stopRangingBeacons(in: region)
//            }
            self.playExhibitAudio(nfcID ?? self.noBeaconID)
//            if (currentMode == DUAL_MODE) {
//                locationManager.startRangingBeacons(in: region)
//            }
        }
    }
    
    func readerSession(_ session: NFCNDEFReaderSession, didInvalidateWithError error: Error) {
        DispatchQueue.main.async {
            // Process detected NFCNDEFMessage objects.
            self.log.text = ""
            self.log.text += "\(error)\n"
            guard let error = error as? NFCReaderError else {return}
            print(error)
            if (error.errorCode == 200) {
                self.setMode(self.PROXIMITY_MODE)
            }
            else {
                DispatchQueue.main.asyncAfter(deadline: DispatchTime.now() + 1.0, execute: {
                    self.nfcSession = NFCNDEFReaderSession(delegate: self, queue: nil, invalidateAfterFirstRead: false)
                    self.nfcSession?.begin()
                })
            }
        }
    }
    
    func logBeacon(_ beacon: CLBeacon) {
        log.text += "Beacon #\(beacon.minor.intValue)\n"
        log.text += "Proximity: \(beacon.proximity == CLProximity.immediate ? "immediate" : "near")\n"
        log.text += "RSSI: \(beacon.rssi)\n"
        log.text += "Accuracy: \(beacon.accuracy.description)\n"
        log.text += "\n"
    }
    
    func getBeaconID(_ beacon: CLBeacon) -> String {
        return "\(beacon.proximityUUID.uuidString).\(beacon.major.intValue).\(beacon.minor.intValue)"
    }
}


extension String {
    //: ### Base64 encoding a string
    func base64Encoded() -> String? {
        if let data = self.data(using: .utf8) {
            return data.base64EncodedString()
        }
        return nil
    }
    
    //: ### Base64 decoding a string
    func base64Decoded() -> String? {
        if let data = Data(base64Encoded: self) {
            return String(data: data, encoding: .utf8)
        }
        return nil
    }
}
