param (
    [string]$CommitDate,
    [string]$CommitMsg,
    [string]$Author
)

$env:GIT_AUTHOR_DATE = $CommitDate
$env:GIT_COMMITTER_DATE = $CommitDate
$env:GIT_AUTHOR_NAME = $Author.Split("<")[0].Trim()
$env:GIT_AUTHOR_EMAIL = $Author.Split("<")[1].Replace(">","").Trim()
$env:GIT_COMMITTER_NAME = $env:GIT_AUTHOR_NAME
$env:GIT_COMMITTER_EMAIL = $env:GIT_AUTHOR_EMAIL

git add .
git commit -m $CommitMsg

Remove-Item Env:\GIT_AUTHOR_DATE
Remove-Item Env:\GIT_COMMITTER_DATE
Remove-Item Env:\GIT_AUTHOR_NAME
Remove-Item Env:\GIT_AUTHOR_EMAIL
Remove-Item Env:\GIT_COMMITTER_NAME
Remove-Item Env:\GIT_COMMITTER_EMAIL
