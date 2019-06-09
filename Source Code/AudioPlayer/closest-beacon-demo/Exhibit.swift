//
//  Exhibit.swift
//  closest-beacon-demo
//
//  Created by Ian Wojtowicz on 2019-05-22.
//  Copyright © 2019 Tevsi LLC. All rights reserved.
//

import Foundation
import UIKit
import CoreLocation
import AVFoundation
import CoreNFC

class Exhibit {
    var color:UIColor
    var soundPath:String
    var beaconUUID:String
    var beaconMajorID:Int
    var beaconMinorID:Int
    var beaconCombinedID:String
    var rangeLimit:Int
    var nfcID:String
    var audioPlayer:AVAudioPlayer
    var stopTask:DispatchWorkItem
    
    init(color:UIColor, soundPath:String, beaconID:[AnyObject], rangeLimit:Int, nfcID:String) throws {
        self.color = color
        self.soundPath = soundPath
        self.beaconUUID = beaconID[0] as! String
        self.beaconMajorID = beaconID[1] as! Int
        self.beaconMinorID = beaconID[2] as! Int
        self.beaconCombinedID = "\(self.beaconUUID).\(self.beaconMajorID).\(self.beaconMinorID)"
        self.rangeLimit = rangeLimit
        self.nfcID = nfcID
        
        do {
            try self.audioPlayer = AVAudioPlayer(contentsOf: NSURL(fileURLWithPath: soundPath) as URL)
            self.audioPlayer.prepareToPlay()
            self.audioPlayer.volume = 0
            self.audioPlayer.numberOfLoops = -1
        }
        catch {
            print(error)
            throw error
        }
        
        self.stopTask = DispatchWorkItem {
            // Do nothing
        }
    }
}
