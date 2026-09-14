use serde::{Deserialize, Serialize};
use std::fmt;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
#[repr(u8)]
pub enum ObjectKind {
    Invalid = 0,
    Ship = 1,
    Blueprint = 2,
    Module = 3,
    Attachment = 4,
    Socket = 5,
    Room = 6,
    Aperture = 7,
    System = 8,
    Decal = 9,
    SymmetryPair = 10,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize, Default)]
#[serde(transparent)]
pub struct StableId(pub u64);

impl StableId {
    pub const INVALID: Self = Self(0);

    pub fn is_valid(self) -> bool {
        self.0 != 0
    }

    pub fn local(document: StableId, kind: ObjectKind, ordinal: u64) -> Self {
        let mut bytes = Vec::with_capacity(24);
        bytes.extend_from_slice(&document.0.to_le_bytes());
        bytes.push(kind as u8);
        bytes.extend_from_slice(&ordinal.to_le_bytes());
        Self(fnv1a64(&bytes).max(1))
    }
}

impl fmt::Display for StableId {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{:016x}", self.0)
    }
}

pub fn fnv1a64(bytes: &[u8]) -> u64 {
    let mut hash = 0xcbf29ce484222325u64;
    for byte in bytes {
        hash ^= u64::from(*byte);
        hash = hash.wrapping_mul(0x100000001b3);
    }
    hash
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn local_ids_are_stable_and_typed() {
        let doc = StableId(0x1234);
        let a = StableId::local(doc, ObjectKind::Module, 1);
        let b = StableId::local(doc, ObjectKind::Module, 1);
        let c = StableId::local(doc, ObjectKind::Socket, 1);
        assert_eq!(a, b);
        assert_ne!(a, c);
        assert!(a.is_valid());
    }
}
