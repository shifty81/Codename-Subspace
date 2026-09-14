use serde::{Deserialize, Serialize};
use std::collections::HashSet;
use subspace_core::StableId;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Room {
    pub id: StableId,
    pub name: String,
    pub pressurized: bool,
    pub deck: i32,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum ApertureKind { Door, Hatch, Airlock, Lift, Ladder }

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Aperture {
    pub id: StableId,
    pub kind: ApertureKind,
    pub room_a: StableId,
    pub room_b: Option<StableId>,
    pub exterior: bool,
}

#[derive(Debug, Clone, Serialize, Deserialize, Default)]
pub struct InteriorLayout {
    pub rooms: Vec<Room>,
    pub apertures: Vec<Aperture>,
}

impl InteriorLayout {
    pub fn validate(&self) -> Vec<String> {
        let room_ids: HashSet<_> = self.rooms.iter().map(|r| r.id).collect();
        let mut errors = Vec::new();
        for aperture in &self.apertures {
            if !room_ids.contains(&aperture.room_a) {
                errors.push(format!("aperture {} room_a missing", aperture.id));
            }
            if let Some(room_b) = aperture.room_b {
                if !room_ids.contains(&room_b) {
                    errors.push(format!("aperture {} room_b missing", aperture.id));
                }
                if room_b == aperture.room_a {
                    errors.push(format!("aperture {} self-connects a room", aperture.id));
                }
            } else if !aperture.exterior {
                errors.push(format!("aperture {} lacks second room but is not exterior", aperture.id));
            }
        }
        errors
    }
}
