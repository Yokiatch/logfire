from __future__ import annotations
from pathlib import Path
from fastapi import APIRouter, HTTPException
from pydantic import BaseModel, Field
from python.bridge.core_wrapper import query

router = APIRouter()


class QueryRequest(BaseModel):
    path: str
    pattern: str = ""
    field_filter: str = ""
    limit: int = Field(default=1000, ge=0)
    offset: int = Field(default=0, ge=0)


class QueryResponse(BaseModel):
    count: int
    lines: list[str]


@router.post("/", response_model=QueryResponse)
def run_query(req: QueryRequest):
    if not Path(req.path).exists():
        raise HTTPException(status_code=404, detail=f"File not found: {req.path}")
    try:
        lines = query(
            req.path,
            pattern=req.pattern,
            field_filter=req.field_filter,
            limit=req.limit,
            offset=req.offset,
        )
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))
    return QueryResponse(count=len(lines), lines=lines)