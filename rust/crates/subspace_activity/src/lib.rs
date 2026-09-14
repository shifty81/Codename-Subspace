use serde::{Deserialize, Serialize};
use std::collections::VecDeque;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum JobState { Queued, Running, Succeeded, Failed, Cancelled }

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ActivityRecord {
    pub id: u64,
    pub label: String,
    pub category: String,
    pub state: JobState,
    pub progress: f32,
    pub status: String,
    pub cancellable: bool,
    pub output: VecDeque<String>,
}

#[derive(Debug, Default)]
pub struct ActivityService {
    next_id: u64,
    records: Vec<ActivityRecord>,
}

impl ActivityService {
    pub fn queue(&mut self, label: impl Into<String>, category: impl Into<String>, cancellable: bool) -> u64 {
        self.next_id = self.next_id.saturating_add(1).max(1);
        let id = self.next_id;
        self.records.push(ActivityRecord {
            id, label: label.into(), category: category.into(), state: JobState::Queued,
            progress: 0.0, status: "Queued".into(), cancellable, output: VecDeque::new(),
        });
        id
    }
    pub fn start(&mut self, id: u64, status: impl Into<String>) -> bool {
        let Some(job) = self.records.iter_mut().find(|r| r.id == id) else { return false; };
        if job.state != JobState::Queued { return false; }
        job.state = JobState::Running; job.status = status.into(); true
    }
    pub fn set_progress(&mut self, id: u64, progress: f32, status: impl Into<String>) -> bool {
        let Some(job) = self.records.iter_mut().find(|r| r.id == id) else { return false; };
        if job.state != JobState::Running { return false; }
        job.progress = progress.clamp(0.0, 1.0); job.status = status.into(); true
    }
    pub fn append_output(&mut self, id: u64, line: impl Into<String>) -> bool {
        let Some(job) = self.records.iter_mut().find(|r| r.id == id) else { return false; };
        const MAX_LINES: usize = 2048;
        while job.output.len() >= MAX_LINES { job.output.pop_front(); }
        job.output.push_back(line.into()); true
    }
    pub fn complete(&mut self, id: u64, success: bool, status: impl Into<String>) -> bool {
        let Some(job) = self.records.iter_mut().find(|r| r.id == id) else { return false; };
        if !matches!(job.state, JobState::Queued | JobState::Running) { return false; }
        job.state = if success { JobState::Succeeded } else { JobState::Failed };
        job.progress = 1.0; job.status = status.into(); true
    }
    pub fn cancel(&mut self, id: u64, status: impl Into<String>) -> bool {
        let Some(job) = self.records.iter_mut().find(|r| r.id == id) else { return false; };
        if !job.cancellable || matches!(job.state, JobState::Succeeded | JobState::Failed | JobState::Cancelled) { return false; }
        job.state = JobState::Cancelled; job.status = status.into(); true
    }
    pub fn records(&self) -> &[ActivityRecord] { &self.records }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn activity_lifecycle_is_explicit() {
        let mut jobs = ActivityService::default();
        let id = jobs.queue("Generate Ship", "Generator", true);
        assert!(jobs.start(id, "Running"));
        assert!(jobs.set_progress(id, 0.5, "Halfway"));
        assert!(jobs.append_output(id, "candidate 1"));
        assert!(jobs.complete(id, true, "Certified"));
        assert_eq!(jobs.records()[0].state, JobState::Succeeded);
    }
}
